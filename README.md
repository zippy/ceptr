# ceptr

Ceptr provides a new computing stack for semantic self-describing data, and protocols.

## Installation

There are currently no dependencies, so just clone this repo.

To get ceptr running on Windows using Eclipse see: https://github.com/zippy/ceptr/blob/master/ceptr-eclipse-csharp-interface.txt

## Usage

## Documentation

You can read the code and API documentation here: http://zippy.github.io/ceptr/

## Architecture

## Streamscapes

## Testing

Currently nothing runs other than a test suite which you can execute simply by typing make from the root directory.  To compile and run all tests simple cd into the root directory and:

    make

## Development

[issue tracking](https://github.com/zippy/ceptr/issue)

[wiki](https://github.com/zippy/ceptr/wiki)

### Contributor Guidelines

* We use test driven development.  Adding a new function or feature, should mean you've added a new test in the spec/ directory.  Our [testing framework] (https://github.com/zippy/ceptr/blob/master/spec/test_framework.h) provides a bunch of C macros that test an expectation with a result, and nicely print broken tests when they fail.  Check out any file in the spec directory to see how it's done.
* We use [Doxygen] (http:/doxygen.org) to generate our API documentation.  Contributors are asked to use doxygen commenting blocks so that we can automatically generate documentation.  Additionally we have the practice of using the @snippet command to include portions of our specs as examples in the API.  We try to make our test cases meaningful so they can be so used and make good examples.

## License

Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al.)

This program is free software: you can redistribute it and/or modify
it under the terms of the license provided in the LICENSE file (GPLv3).

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

## Acknowledgements
