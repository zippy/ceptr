#!/usr/local/bin/perl

# This file generates c code that defines system semantic definitions.
# it reads the file "base_defs" as a source file for creating the definitions
#
# Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).

use strict;
use warnings;
use Data::Dumper;

my $defs_file = 'src/base_defs';
open(my $fh,'<:encoding(UTF-8)', $defs_file)
        or die "Could not open definitions source file '$defs_file' $!";
my $defs_c_file = 'src/base_defs.c';
open(my $cfh,'>:encoding(UTF-8)', $defs_c_file)
        or die "Could not open definitions c file '$defs_c_file' $!";
my $defs_h_file = 'src/base_defs.h';
open(my $hfh,'>:encoding(UTF-8)', $defs_h_file)
        or die "Could not open definitions header file '$defs_h_file' $!";

my %c;
my @d;
my %fmap = ('Structure'=>'sT','StructureS'=>'sTs','Symbol'=>'sY','Process'=>'sP','SetSymbol'=>'sYs');
my $context = "SYS";
my %declared;
my %anon;

sub addDef {
    my $type = shift;
    my $context = shift;
    my $name = shift;
    my $def = shift;
    my $def_type = ($type eq 'Structure' && $def =~ /sT_/) ? "StructureS" : $type;
    push @d,[$def_type,$context.'_CONTEXT',$name,$def];

    # don't need to redo header defs stuff for just setting symbols definition
    if ($type ne 'SetSymbol') {
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

sub andify {
    my $n = shift;
    join("_AND_",split(/,/,$n))
}
sub makeName {
    my $n = shift;
    $n =~ s/sT_SYM\(([^)]+)\)/$1/g;
    $n =~ s/sT_SET\([0-9]+,/ONE_OF_/g;
    $n =~ s/sT_SEQ\(2,([^,]+),\g1/PAIR_OF_$1/g;
    $n =~ s/sT_SEQ\(2,([^)]+)\)/TUPLE_OF_$1/g;
    $n =~ s/sT_SEQ\([0-9]+,/LIST_OF_/g;
    $n =~ s/sT_OR/LOGICAL_OR_OF_/g;
    $n =~ s/sT_STAR/ZERO_OR_MORE_OF_/g;
    $n =~ s/sT_PLUS/ONE_OR_MORE_OF_/g;
    $n =~ s/sT_QMRK/ZERO_OR_ONE_OF_/g;
    $n =~ s/[()]//g;
    return &andify($n);
}

sub buildNumParams {
    my $x = shift;
    my @params = split /,/,$x;
    return scalar(@params).";".join(';',@params);
}

sub convertStrucDef {
#    my @tokens = split /([,\(\)\{\}\|\?\+\*])/,shift;
    my $x= shift;
    #for a simple structure without optionality we'll just use sT with params
    if (!($x=~/[\{\}\(\)\+\*?|]/)) {
        $x = &buildNumParams($x);
    }
    else {
        $x =~ s/([a-zA-Z0-9_]+)/sT_SYM<$1>/g;
        while ($x=~/\|/) {
            $x =~ s/\|\[([^|\]]+)\|([^\]]+)\]/sT_OR<$1;$2>/;
        }
        while ($x=~/[,()\{\}?+*]/) {
            $x =~ s/\*([a-zA-Z0-9_<>;]+)/sT_STAR<$1>/g;
            $x =~ s/\+([a-zA-Z0-9_<>;]+)/sT_PLUS<$1>/g;
            $x =~ s/\?([a-zA-Z0-9_<>;]+)/sT_QMRK<$1>/g;
            $x =~ s/\(([^()]+)\)/"sT_SEQ<".&buildNumParams($1).'>'/eg;
            $x =~ s/\{([^\}\{]+)\}/"sT_SET<".&buildNumParams($1).'>'/eg;
        }
        $x =~ s/</(/g;
        $x =~ s/>/)/g;
    }
    $x =~ s/;/,/g;
    return $x;
}

while (my $def = <$fh>) {
    chomp $def;
    next if ($def =~ /^ *#/);       # ignore comments
    next if ($def) =~ /^[ \t]*$/;   #ignore whitespace lines
    if ($def =~ /(.*): *(.*);(.*)/) {
        my $type = $1;
        if ($type eq 'Context') {$context = $2;next;}
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
                &addDef($type,$context,$name,$structure);
            }
            elsif ($type eq 'Structure') {
                $params =~ /(.*?),(.*)/;
                my $name = $1;
                my $structure_def = $2;
                &addDef($type,$context,$name,&convertStrucDef($structure_def));
            }
            elsif ($type eq 'Process') {
                $params =~ /(.*?),(.*)/;
                my $name = $1;
                my $process_def = $2;
                &addDef($type,$context,$name,$process_def);
            }
        }

    } else {
        die "unable to parse $def";
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
 * NOTE: this file is auto-generated by base_defs.pl
 *
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#include "base_defs.h"
#include "sys_defs.h"
#include "def.h"
#include "process.h"

void base_defs() {
EOF
foreach my $s (@d) {
    my @x = @$s;
    my $n = shift @x;
    my $p = join(',',@x);
    print $cfh "  $fmap{$n}($p);\n";
}

print $cfh <<EOF;
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
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#ifndef _CEPTR_BASE_DEFS_H
#define _CEPTR_BASE_DEFS_H
#include "sys_defs.h"

void base_defs();
EOF

&hout("SYS","Symbol");
&hout("SYS","Structure");
&hout("SYS","Process");
&hout("TEST","Symbol");
&hout("LOCAL","Symbol");
&hout("LOCAL","Structure");

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
        print $hfh '#define '.$s." G_contexts[$context"."_CONTEXT].".lc($types).'['.$s."_ID]\n";
    }
}
print $hfh <<EOF;

#endif
EOF
