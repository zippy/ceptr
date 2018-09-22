# Ceptr: The fabric of the new crypto-semantic Internet

Ceptr provides a new computing stack for semantic self-describing data and protocols, which enables a new kind recomposable medium for distributed social computing.

## Project Status

It may look like the Ceptr project is on hiatus, but it is **alive and well**. We pulled out a small part of Ceptr (synchronization for multi-instance receptors) and are going head-to-head with blockchain as an alternative approach to building distributed sytems. That portion of of Ceptr is called [Holochain](https://github.com/metacurrency/holochain) and is quite active with new [distributed applications](https://github.com/holochain/apps) being developed for it regularly.

We will return to integrating the other parts of Ceptr (like low-level semantics and self-describing protocols for easy interoperability) soon. However, we may scrap this codebase in C and rebuild it Go. Holochain is getting a lot more traction in Go, than Ceptr got written in C.

## Installation

There are currently no dependencies, so just clone this repo.

Then cd into the root directory of the repo and type:

    make ceptr

This will build a ceptr node running the command-line receptor, which is currently very primitive, with just four commands "time" (which returns a Tick semantic tree) "receptors" (which lists the currently active receptors) "debug" (which toggles debug mode) and "quit". more coming soon!

For some guidance on getting the code-base to compile on Windows using Eclipse see: https://github.com/zippy/ceptr/blob/master/ceptr-eclipse-csharp-interface.txt  (Note: this hasn't been tested recently and is no longer in sync with the c codebase...)

## Usage

TBD...

Also, see "Testing" below.

## Documentation

You can read the code and API documentation here: http://zippy.github.io/ceptr/

Or watch a brief overview webinar/video here: https://www.youtube.com/watch?v=3Db-8lD1lNA

## Architecture

For a broad overview of the project see [the Ceptr Revelation] (http://ceptr.org/revelation/)

Or check out this prezi: https://prezi.com/raptqxuputwp/ceptr-tech-overview/

Or this longer presentation of that prezi: https://www.youtube.com/watch?v=Y9ZhswdOm14

## Testing

To compile and run all the tests simply cd into the root directory and type:

    make test

This runs all of the tests, which live in the specs sub-directory.

The tests also produce a bunch of semantic trees dumped out to json for visualization in a testing web-app.  To be able to see these, you need to place the contents of the "web" sub-directory into a web server somewhere, and then just point your browser to that directory.  Please note that this isn't a comprehensive testing environment, just a space we've used for creating visuals for presentation purposes.

## Development

[![Join the dev chat at https://gitter.im/zippy/ceptr](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/zippy/ceptr?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge) [![In Progress](https://badge.waffle.io/zippy/ceptr.svg?label=in%20progress&title=In%20Progress)](http://waffle.io/zippy/ceptr)

We welcome participation. Check our our waffle for [Roadmap & kanban](https://waffle.io/zippy/ceptr) or if you prefer you can just use github's [issue tracking](https://github.com/zippy/ceptr/issues).

If you have any dev questions pop them into our [dev gitter](https://gitter.im/zippy/ceptr).

Finally, zippy314 does some [livecoding](https://www.livecoding.tv/zippy/)..

### Contributor Guidelines

#### Tech

* We use test driven development.  Adding a new function or feature, should mean you've added a new test in the spec/ directory.  Our [testing framework] (https://github.com/zippy/ceptr/blob/master/spec/test_framework.h) provides a bunch of C macros that test an expectation with a result, and nicely print broken tests when they fail.  Check out any file in the spec directory to see how it's done.
* We use [Doxygen] (http:/doxygen.org) to generate our API documentation.  Contributors are asked to use doxygen commenting blocks so that we can automatically generate documentation.  Additionally we have the practice of using the @snippet command to include portions of our specs as examples in the API.  We try to make our test cases meaningful so they can be so used and make good examples.
* Elements of Style: no tabs indentation (4 spaces).  Note: if you are using emacs with prelude it specifically sets indent-tabs-mode to 't' in modules/prelude-c.el for c-mode so you have to fix that there.

#### Social

* Protocols for Inclusion.

We recognize the need to actively foster vibrant thriving community, which means among other things, building a culture that breaks cycles of marginalization and dominance behavior.  To that end many open source communities adopt Codes of Conduct like [this one](http://contributor-covenant.org/version/1/3/0/).  We are in the process of addressing the goals of such codes in what we feel is a more general way, by establishing meta requirements for each membrane within our social organism to describe its <i>Protocols for Inclusion</i>.  Until we have done so please operate using the above referenced code as a general protocol for inclusion.

## License

Copyright (C) 2013-2016, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al.)

This program is free software: you can redistribute it and/or modify
it under the terms of the license provided in the LICENSE file (GPLv3).

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

## Acknowledgements
