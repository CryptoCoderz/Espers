Espers [ESP] 2016-2024 integration/staging tree
===============================================


Project Website
-------------------------
https://espers.io/


Navigation of these documents
--------------------------
- General Readme (You are here)
- [Latest Releases](https://github.com/CryptoCoderz/Espers/releases)
- [Features/Specifications](doc/README.md)
- [Installation Guide](INSTALL.md)
- [Asset Attributions](doc/assets-attribution.md)


License
-------
Espers [ESP] is released under the terms of the MIT license. See [COPYING](COPYING) for more
information or see https://opensource.org/licenses/MIT.

Assistance and Contact
--------------------
We strive to be an open and friendly community that is eager to help each other through 
the journey of the Espers [ESP] project development. If at any time someone needs assistance
or just a community member to talk to, please feel free to join our Discord at https://discord.gg/cn3AfPS

Development Process
-------------------
The `master` branch is regularly built and tested, but is not guaranteed to be
completely stable. [Tags](https://github.com/CryptoCoderz/Espers/tags) are created
regularly to indicate new official, stable release versions of Espers [ESP].

The contribution workflow is described in [CONTRIBUTING.md](CONTRIBUTING.md).

The developer Discord should be used to discuss complicated or controversial changes 
before working on a patch set.

Developer Discord can be found at https://discord.gg/cn3AfPS .

Testing
-------
Testing and code review is the bottleneck for development; we get more development tasks
than we can review, build, and test on short notice. Please be patient and help out by testing
other people's pull requests and our test builds as they become available.
Remember this is a security-critical project where any mistake might cost people
the loss of their Espers which is something we strive to prevent at all costs.

### Automated Testing

Developers are strongly encouraged to write [unit tests](/doc/unit-tests.md) for new code, and to
submit new unit tests for old code. Unit tests can be compiled and run
(assuming they weren't disabled in configure) with: `make check`

### Manual Quality Assurance (QA) Testing

Changes should be tested by somebody other than the developer who wrote the
code. This is especially important for large or high-risk changes. It is useful
to add a test plan to the pull request description if testing the changes is
not straightforward.
