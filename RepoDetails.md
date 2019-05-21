# Project Mu Oem Sample Repository

??? info "Git Details"
    Repository Url: {{mu_oem_sample.url}}  
    Branch:         {{mu_oem_sample.branch}}  
    Commit:         [{{mu_oem_sample.commit}}]({{mu_oem_sample.commitlink}})  
    Commit Date:    {{mu_oem_sample.date}}

This repository is considered sample code for any entity building devices using Project Mu.  It is likely that any device manufacturer will want to customize the device behavior by changing the modules in this package.  

* Numerous libraries to support UEFI Boot Device Selection phase (BDS) 
* Firmware Version information
* UI App / "Frontpage" application support as well as example

## More Info

FrontpageDsc and FrontpageFdf that can be included so you don't have to unravel all of the libraries and protocols that are required to get started with FrontPage. A brief description of each MS component was added to FrontPageDsc to help explain how each piece of the puzzle fits in.

Please see the Project Mu docs (https://github.com/Microsoft/mu) for more information.  

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).

For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

## Per Platform Libraries

MsPlatformDevicesLib, DfciDeviceIdSupportLib, PlatformThemeLib.
These three libraries need to be implemented per platform. An example can be found in the [NXP iMX8 platform](https://github.com/ms-iot/MU_PLATFORM_NXP).

## Issues

Please open any issues in the Project Mu GitHub tracker. [More Details](https://microsoft.github.io/mu/How/contributing/)

## Contributing Code or Docs

Please follow the general Project Mu Pull Request process.  [More Details](https://microsoft.github.io/mu/How/contributing/)

* [Code Requirements](/DeveloperDocs/code_requirements)
* [Doc Requirements](/DeveloperDocs/doc_requirements)

## PR-Gate Builds

```cmd
pip install --upgrade -r requirements.txt
mu_build -c corebuild.mu.json
```

[![Build Status](https://dev.azure.com/projectmu/mu/_apis/build/status/mu_oem_sample%20PR%20gate)](https://dev.azure.com/projectmu/mu/_build/latest?definitionId=7)


## Copyright & License

Copyright (c) 2016-2018, Microsoft Corporation

All rights reserved. Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
