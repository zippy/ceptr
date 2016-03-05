#!/usr/local/bin/perl

# This script generates c code that defines system semantic definitions.
# It reads the file "base_defs" as a source file for creating the definitions.
# It also produces html output documentation of the various definitions for
# use in our doxygen based docs.
#
# Copyright (C) 2013-2016, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).

use strict;
use warnings;
use Data::Dumper;

my $fh = openf('<','src/base_defs');
my $cfh = openf('>','src/base_defs.c');
my $hfh = openf('>','src/base_defs.h');

sub openf {
    my $rw = shift;
    my $fn = shift;
    open(my $fh,"$rw:encoding(UTF-8)", $fn)
            or die "Could not open '$fn' $!";
    return $fh
}

my %c;
my @d;
my %fmap = ('Structure'=>'sT','StructureS'=>'sTs','Symbol'=>'sY','Process'=>'sP','SetSymbol'=>'sYs','Protocol'=>'sProt','Data'=>'sData','Label'=>'sLabel');
my $context = "SYS";
my %declared;
my %anon;
my %comments;
my %declmap;
my %global_data;
my @contexts;

sub addDef {
    my $type = shift;
    my $context = shift;
    my $name = shift;
    my $def = shift;
    my $comment = shift;
    my $def_type = ($type eq 'Structure' && $def =~ /sT_/) ? "StructureS" : $type;
    $declmap{$name} = scalar(@d);
    push @d,[$def_type,$context.'_CONTEXT',$name,$def];

    $comments{$name} = $comment;

    if ($type eq 'SetSymbol') {
        # if this is the setting of the value of a symbol that was previously
        # declared, save the real definition for docs purposes
        $declared{$name} = $def;
    }
    else {
        if (! exists $c{$context}) {
            $c{$context} = {};
        }

        my $defs = $c{$context};

        if (! exists $defs->{$type}) {
            $defs->{$type} = [];
        }

        my $a = $defs->{$type};
        push @$a, $name;
    }
}

sub makeName {
    my $n = shift;
    $n =~ s/sT_SYM\(([^)]+)\)/$1/g;
    $n =~ s/sT_SEQ\(2,([^,]+),\g1/PAIR_OF_$1/g;
    $n =~ s/sT_SEQ\(2,([^)]+)\)/TUPLE_OF_$1/g;
    $n =~ s/sT_SEQ\([0-9]+,/LIST_OF_/g;
    $n =~ s/sT_OR\([0-9]+,/LOGICAL_OR_OF_/g;
    $n =~ s/sT_STAR/ZERO_OR_MORE_OF_/g;
    $n =~ s/sT_PLUS/ONE_OR_MORE_OF_/g;
    $n =~ s/sT_QMRK/ZERO_OR_ONE_OF_/g;
    $n =~ s/sT_PCNT/STRUCTURE_OF_/g;
    $n =~ s/sT_BANG/ANY_SYMBOL/g;
    $n =~ s/[()]//g;
    $n =  join('_AND_',split(/,/,$n));
    $n =  join('_OR_',split(/\|/,$n));
    return $n;
}

sub buildNumParams {
    my $x = shift;
    my $sep = shift;
    my @params = split /$sep/,$x;
    return scalar(@params).";".join(';',@params);
}

sub convertStrucDef {
#    my @tokens = split /([,\(\)\{\}\|\?\+\*])/,shift;
    my $x= shift;
    #for a simple structure without optionality we'll just use sT with params
    if (!($x=~/[\{\}\(\)\+\*?|\%\!]/)) {
        $x = &buildNumParams($x,',');
    }
    else {
        $x =~ s/([a-zA-Z0-9_]+)/sT_SYM<$1>/g;
        $x =~ s/\%sT_SYM(<[a-zA-Z0-9_]+>)/sT_PCNT$1/g;
        while ($x=~/[,()\{\}?+*!]/) {
            $x =~ s/\*([a-zA-Z0-9_<>;]+)/sT_STAR<$1>/g;
            $x =~ s/\+([a-zA-Z0-9_<>;]+)/sT_PLUS<$1>/g;
            $x =~ s/\?([a-zA-Z0-9_<>;]+)/sT_QMRK<$1>/g;
            $x =~ s/\(([^()]+)\)/"sT_SEQ<".&buildNumParams($1,',').'>'/eg;
            $x =~ s/\|\{([^\{\}]+)\}/"sT_OR<".&buildNumParams($1,'\|').'>'/eg;
            $x =~ s/!/sT_BANG/g;
        }
        $x =~ s/</(/g;
        $x =~ s/>/)/g;
    }
    $x =~ s/;/,/g;
    return $x;
}

my $def = "";
while (my $line = <$fh>) {
    chomp $line;
    next if ($line =~ /^ *#/);       # ignore comments
    next if ($line =~ /^[ \t]*$/);   # ignore whitespace lines
    if ($line =~ /^\s*([^;]*)\s*$/) { # if line ends has no semicolon assume it continues on the next line
        $def .= $1;
        next;
    }
    $line =~ s/^\s+|\s+$//;  #trim leading and trailing whitespace
    $def .= $line;
#    print "testing $def\n";
    if ($def =~ /(.*): (.*?);(.*)/) {
        $def = "";
        my $type = $1;
        if ($type eq 'Context') {
            $context = $2;
            push @contexts,$context;
            next;
        }
        my $params = $2;
        my $comment = $3;

        if ($type eq 'Declare') {
            my @symbols = split /,/,$params;
            foreach my $s (@symbols) {
                $declared{$s} = 1;
                &addDef("Symbol",$context,$s,"NULL_STRUCTURE");
            }
        }
        else {
            if ($type eq 'Symbol') {
                $params =~ /(.*?),(.*)/;
                my $name = $1;
                my $structure = $2;
                if ($declared{$name}) {
                    $type = "SetSymbol";
                }
                if ($structure =~ /^\[(.*)\]$/) {
                    my $sdef = &convertStrucDef($1);
                    my $sname = &makeName($sdef);
                    if (!$anon{$sname}) {
                        $anon{$sname} = 1;
                        &addDef("Structure",$context,$sname,$sdef);
                    }
                    $structure = $sname;
                }
                &addDef($type,$context,$name,$structure,$comment);
            }
            elsif ($type eq 'Structure') {
                $params =~ /(.*?),(.*)/;
                my $name = $1;
                my $structure_def = $2;
                &addDef($type,$context,$name,&convertStrucDef($structure_def),$comment);
            }
            elsif (($type eq 'Process') || ($type eq 'Protocol') || ($type eq 'Label') ) {
                $params =~ /(.*?),(.*)/;
                my $name = $1;
                my $def = $2;
                &addDef($type,$context,$name,$def,$comment);
            }
            elsif ($type =~ /(.*)Data/) {
                my $data_type = $1;
                $type = 'Data';
                $params =~ /(.*?),(.*)/;
                my $name = $1;
                my $def = $2;
                $global_data{$name} = 1 if ($data_type eq 'Global');

                while ($def =~ s/\(([a-zA-Z0-9_]+):/($1,/g) {};
                #while ($def =~ s/ *\(([^(]+):([^)]+)\)/($1,$2)/g) {} ;
                $def =~ s/ *\(/,STX_OP,/g;
                $def =~ s/\)/,STX_CP,/g;
                #$def =~ s/ \(([^(]+):([^)]+)\)/,$1,$2,/g;
                #$def =~ s/\)/,NULL_SYMBOL/g;
                #$def =~ s/ \(/,/g;
                $def =~ s/,,/,/g;
                $def =~ s/^,(.*),$/$1/g;
                $def =~ s/\/([0-9\/]+)/$1,TREE_PATH_TERMINATOR/g;
                $def =~ s/\/([0-9])/,$1/g;
                #     $def =~ s/^\((.*)/$1/;
                &addDef($type,$context,$name,$def,$comment);
            } else {
                die "unknown directive: $type";
            }
        }
    } else {
        die "unable to parse $line";
    }
}
#print Dumper(\@d);
#print $fmap{'Symbol'};

print $cfh <<'EOF';
/**
 * @ingroup def
 *
 * @file base_defs.c
 * @brief auto-generated system definitions
 *
 * NOTE: this file is auto-generated by base_defs.pl so DON'T EDIT IT MANUALLY!
 *
 * @copyright Copyright (C) 2013-2016, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#include "base_defs.h"
#include "sys_defs.h"
#include "def.h"
#include "process.h"
#include "receptor.h"
#include "protocol.h"

EOF

sub makeRecptorName {
    my $CTX = shift;
    my $ctx_name = $CTX;
    if (($CTX eq 'SYS') || ($CTX eq 'TEST') || $CTX eq 'CLOCK') {
        $ctx_name .= '_RECEPTOR';
    }
    return $ctx_name;
}


foreach my $CTX (@contexts) {
    my $ctx_name = &makeRecptorName($CTX);
    print $cfh "SemanticID $ctx_name={0,0,0};\n";
}

foreach my $s (@d) {
    my @x = @$s;
    next if ($x[0] eq 'SetSymbol');
    next if ($x[0] eq 'Data');
    next if ($x[0] eq 'Label');
    print $cfh "SemanticID $x[2]={0,0,0};\n";
}

print $cfh "\n";


print $cfh "void base_defs(SemTable *sem) {\n";
foreach my $s (@d) {
    my @x = @$s;
    my $n = shift @x;
    my $p = join(',',@x);
    print $cfh "  $fmap{$n}($p);\n";
    if ($n eq 'Data' && $global_data{$x[1]}) {
        printf $cfh "  G_$x[1] = $x[1];\n";
    }
}
print $cfh "}\n\n";

my $ctx_code ="";
foreach my $CTX (@contexts) {
    my $ctx = lc($CTX);
    my $ctx_name = &makeRecptorName($CTX);

    $ctx_code .= "    $ctx_name = _d_define_receptor(sem,\"$ctx_name\",__r_make_definitions(),SYS_CONTEXT);\n";
}

print $cfh <<"EOF";
void base_contexts(SemTable *sem) {
$ctx_code
}
EOF

print $hfh <<'EOF';
/**
 * @ingroup def
 *
 * @file base_defs.h
 * @brief auto-generated system definitions
 *
 * NOTE: this file is auto-generated by base_defs.pl
 *
 * @copyright Copyright (C) 2013-2016, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#ifndef _CEPTR_BASE_DEFS_H
#define _CEPTR_BASE_DEFS_H
#include "sys_defs.h"

void base_defs(SemTable *sem);
void base_contexts(SemTable *sem);
EOF

my @ctxe;
foreach my $CTX (@contexts) {
    push @ctxe,$CTX."_CONTEXT";
    my $ctx_name = &makeRecptorName($CTX);
    print $hfh "SemanticID $ctx_name;\n";
}

print $hfh "\nenum SemanticContexts {".join(',',@ctxe).",_NUM_DEFAULT_CONTEXTS};\n";

foreach my $CTX (@contexts) {
    &hout($CTX,"Symbol");
    &hout($CTX,"Structure");
    &hout($CTX,"Process");
    &hout($CTX,"Protocol");
}

sub camelify {
    my $s = shift;
    my @n = split(/_/,$s);
    @n = map {ucfirst(lc($_))} @n;
    $n[0] = lc($n[0]);
    return join('',@n);
}

sub printEnum {
    my $hfh = shift;
    my $s = shift;
    my $d = shift;
    if ($d ne 'NULL_SYMBOL') {
        print $hfh 'enum '.$s.'Indexes {';
        my $n = &camelify($s);
        my @i = split(/,/,$d);
        @i = map {$n.ucfirst(&camelify($_)).'Idx'} @i;
        $i[0].="=1";
        my $idx = join(',',@i);
        print $hfh $idx;
        print $hfh "};\n";
    }
}

# add definitions to the header file
sub hout {
    my $context = shift;
    my $type = shift;
    my $types = $type eq "Process" ? "Processes" : $type."s";

    my $defs = $c{$context};
    my $a = $defs->{$type};
    print $hfh <<EOF;

/**********************************************************************************/
// $context:$type
enum $context${\($type)}IDs {
    NULL_${\(($context ne 'SYS' ? $context.'_' : '').uc($type))}_ID,
EOF
    foreach my $s (@$a) {
        print $hfh '    '.$s."_ID,\n";

    }
    print $hfh '    NUM_'.$context.'_'.uc($types)."\n};\n";
    foreach my $s (@$a) {
        print $hfh "SemanticID $s;\n";
    }
    # attempt to create enums for all structure Indexes.  doesn't quite work
    # if ($type eq 'Structure') {
    #     foreach my $s (@$a) {
    #         my $def = $d[$declmap{$s}][3];
    #         if ($def =~ /^[0-9],(.*)$/) {
    #             printEnum($hfh,$s,$1);
    #         }
    #         $def =~ s/sT_SYM\(([^)]+)\)/$1/g;
    #         if ($def =~ /sT_SEQ\([0-9],([^(]*)\)(.*)$/) {
    #             print $1,"\n";
    #             print "END: $2\n";
    #             printEnum($hfh,$s,$1);
    #         }
    #     }
    # }
}

## add global data to the header file
foreach my $s (keys %global_data) {
    print $hfh "\n//Global data from base_defs\n";
    print $hfh 'T * G_'.$s.";\n";
}

print $hfh <<EOF;

#endif
EOF

#generate sys process documentation file
my $pdfh = openf('>','doxy/sys_processes.html');
my $phtml = << 'HTML';
<style type="text/css">table.processes ol{margin:0;padding:1em;}</style>
<table class="doxtable processes"><tr><th>Process</th><th>Children (inputs)</th><th>Reduces To (output)</th><th>Comments</th></tr>
HTML
my $stdfh = openf('>','doxy/sys_structures.html');
my $sthtml = << 'HTML';
<table class="doxtable"><tr><th>Structure</th><th>Defintion</th></th><th>Comments</th></tr>
HTML
$sthtml .= "<tr><td><a name=\"NULL_STRUCTURE\"></a>NULL-STRUCTURE</td><td><i>undefined</i></td><td></td></tr>\n";

my $sydfh = openf('>','doxy/sys_symbols.html');
my $syhtml = << 'HTML';
<table class="doxtable"><tr><th>Symbol</th><th>Semantic use of Structure:</th><th>Comments</th></tr>
HTML
$syhtml .= "<tr><td><a name=\"NULL_SYMBOL\"></a>NULL_SYMBOL</td><td><i>undefined</i></td><td></td></tr>\n";

my $sysdfh = openf('>','doxy/sys_defs.html');
my $sysdhtml = <<"HTML";
<style type="text/css">
 div.def-type {display:inline-block;}
 div.def-item {display:inline-block;}
 div.def-name {display:inline-block;}
 div.def-sig-in {display:inline-block;}
 div.def-sig-out {display:inline-block;}
 div.def-sym-def {display:inline-block;}
 div.def-struc-def {display:inline-block;}
 div.def-protocol-def {display:inline-block;}
 div.def-sig-out {display:inline-block;}
 div.def-comment {display:inline-block;}
</style>

HTML
my $cur_ctx = "";
foreach my $s (@d) {
    my @x = @$s;
    my $type = $x[0];
    my $context = $x[1];
    my $name = $x[2];
    my $def = $x[3];
    if ($cur_ctx ne $context) {
        if ($cur_ctx ne "") {$sysdhtml .= "</div>\n"};
        $sysdhtml .= "<div class=\"defs-context\">\n   <h4>$context</h4>\n";
        $cur_ctx = $context;
    }
    if ($type eq 'Process') {
        my ($code,$desc,$out,$out_type,$out_sym,@def) = split /,/,$def;
        $desc =~ /"(.*)"/;
        $desc = $1;
        $out = &processSig($out,$out_type,$out_sym);
        my $in = "";
        while ($def[0] && $def[0] ne '0') {
            my $n = shift @def;
            my $optional = 0;
            if ($def[0] eq 'SIGNATURE_OPTIONAL') {
                $optional = 1;
                shift @def;
            }
            my $i = &processSig($n,shift @def,shift @def);
            $i = "[$i]" if $optional;
            $in .= "<li>$i</li>";
        }
        my $c = "<i>$desc</i>";
        $c = "$c<br />".$comments{$name} if $comments{$name};
        $phtml .= "<tr><td><a name=\"$name\"></a>$name</td><td><ol>$in</ol></td><td>$out</td><td>$c</td></tr>\n";
        $sysdhtml .= << "HTML";
   <div class="def-item def-process">
       <div class="def-type">Process:</div>
       <div class="def-name"><a name="$name"></a>$name</div>
       <div class="def-sig-in">$in</div>
       <div class="def-sig-out">$out</div>
       <div class="def-comment">$c</div>
   </div>
HTML

    }
    elsif ($type eq 'Symbol') {
        if ($declared{$name}) {
            $def =$declared{$name};
        }
        my $n = $def;
        $def =~ s/_/-/g; # we do this so long defs will wrap in html cus underscores don't
        $def = "<a href=\"ref_sys_structures.html#$n\">$def</a>";
        my $c = $comments{$name} ? $comments{$name} : "";
        $syhtml .= "<tr><td><a name=\"$name\"></a>$name</td><td>$def</td><td>$c</td></tr>\n";
        $sysdhtml .= << "HTML";
   <div class="def-item def-symbol">
       <div class="def-type">Symbol:</div>
       <div class="def-name"><a name="$name"></a>$name</div>
       <div class="def-sym-def">$def</div>
       <div class="def-comment">$c</div>
   </div>
HTML
    }
    elsif ($type eq 'Structure') {
        my ($count,@def) = split /,/,$def;

        for(@def) {
            s/(.*)/<a href="ref_sys_symbols.html#$1">$1<\/a>/;
        }

        $def = join(', ',@def);
        $def = "SEQ($def)" if (scalar @def > 1);
        $sthtml .= "";
        my $c = $comments{$name} ? $comments{$name} : "";
        $sthtml .= "<tr><td><a name=\"$name\"></a>$name</td><td>$def</td><td>$c</td></tr>\n";
        $sysdhtml .= << "HTML";
   <div class="def-item def-structure">
       <div class="def-type">Structure:</div>
       <div class="def-name"><a name="$name"></a>$name</div>
       <div class="def-struc-def">$def</div>
       <div class="def-comment">$c</div>
   </div>
HTML
    }
    elsif ($type eq 'StructureS') {
        my $n = $name;
        $n =~ s/_/-/g;
        $def =~ s/sT_//g;
        $def =~ s/,/, /g;
        $def =~ s/SYM\((.*?)\)/<a href="ref_sys_symbols.html#$1">$1<\/a>/g;
        $def =~ s/PCNT\((.*?)\)/%<a href="ref_sys_structures.html#$1">$1<\/a>/g;
        $def =~ s/(SEQ|OR)\([0-9]+, /$1(/g;
        $def =~ s/STAR/\*/g;
        $def =~ s/PLUS/\+/g;
        $def =~ s/QMRK/\?/g;
        $def =~ s/BANG/\!/g;
        my $c = $comments{$name} ? $comments{$name} : "";
        $sthtml .= "<tr><td><a name=\"$name\"></a>$n</td><td>$def</td><td>$c</td></tr>\n";
        $sysdhtml .= << "HTML";
   <div class="def-item def-structure">
       <div class="def-type">Structure:</div>
       <div class="def-name"><a name="$name"></a>$n</div>
       <div class="def-struc-def">$def</div>
       <div class="def-comment">$c</div>
   </div>
HTML
    } elsif ($type eq 'Protocol') {
        my $c = $comments{$name} ? $comments{$name} : "";
        $sysdhtml .= << "HTML";
   <div class="def-item def-protocol">
       <div class="def-type">Protocol:</div>
       <div class="def-name"><a name="$name"></a>$name</div>
       <div class="def-protocol-def">$def</div>
       <div class="def-comment">$c</div>
   </div>
HTML
    }
}
print $sysdfh $sysdhtml;

&finish($phtml,$pdfh);
&finish($sthtml,$stdfh);
&finish($syhtml,$sydfh);

sub finish {
    my $html = shift;
    my $fh = shift;
    $html .= << 'HTML';
</table>
HTML
    print $fh $html;
}

sub processSig {
    my $name = shift;
    my $type = shift;
    my $sym = shift;
    $name =~ /"(.*)"/;$name = $1;
    $type =~ /SIGNATURE_(.*)/;$type = $1;
    $sym =~ s/(.*)/<a href="ref_sys_symbols.html#$1">$1<\/a>/;

    return "$name($type:$sym)";
}
