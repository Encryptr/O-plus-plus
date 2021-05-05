# Opp Interpreter
> Opp interpreter, an interpreter aiming to be fully compliant with C89

[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![License][license-shield]][license-url]

The Opp Interpreter was created out of the need for a fast, small, and simple interpreter for the C programming language. Its goal is to be able to take any C89 (plus some additions) C file and be able to run it as an interpreter. This may be useful when needing to prototype and not wanting to go through the whole compilation process or when wanting to safely play around with some C features. In the end, there are many great uses for a C interpreter and Opp aims to be able to cover as many of them as possible.

<img src="logo.png" width="350" height="350"/>

## Installation

OS X & Linux:

```sh
# Run the makefile
make
```

Windows:

```sh
# Make sure to have mingw
mingw32-make.exe 
```

## Usage example

Example C program.

```c
int main(int argc, char** argv) {
	printf("Hello World!\n");
	return 0;
}
```

## Development
> Opp is being developed by **Encryptr**, a open-source organization. Becoming a dev/contributor is as easy as joining Encryptr's discord server. [Link](https://discord.gg/8tcf6sJ5Gq)

## Release History
* 0.0.1
    * Work in progress

## Meta

Distributed under the Apache-2.0 license. See ``LICENSE`` for more information.

[https://github.com/Encryptr/O-plus-plus](https://github.com/Encryptr/O-plus-plus)

## Contributing

1. Fork it (<https://github.com/Encryptr/O-plus-plus/fork>)
2. Create your feature branch (`git checkout -b feature/fooBar`)
3. Commit your changes (`git commit -am 'Add some fooBar'`)
4. Push to the branch (`git push origin feature/fooBar`)
5. Create a new Pull Request

<!-- Markdown link & img dfn's -->
[wiki]: https://github.com/Encryptr/O-plus-plus/wiki
[contributors-shield]: https://img.shields.io/github/contributors/Encryptr/O-plus-plus.svg?style=flat-square
[contributors-url]: https://github.com/Encryptr/O-plus-plus/graphs/contributors

[forks-shield]: https://img.shields.io/github/forks/Encryptr/O-plus-plus.svg?style=flat-square
[forks-url]: https://github.com/Encryptr/O-plus-plus/network/members

[stars-shield]: https://img.shields.io/github/stars/Encryptr/O-plus-plus.svg?style=flat-square
[stars-url]: https://github.com/Encryptr/O-plus-plus/stargazers

[issues-shield]: https://img.shields.io/github/issues/Encryptr/O-plus-plus.svg?style=flat-square
[issues-url]: https://github.com/Encryptr/O-plus-plus/issues

[license-shield]: https://img.shields.io/github/license/Encryptr/O-plus-plus.svg?style=flat-square
[license-url]: https://github.com/Encryptr/O-plus-plus/blob/master/LICENSE.txt


