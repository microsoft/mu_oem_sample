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

Please see the Project Mu docs (<https://github.com/Microsoft/mu>) for more
information.  

This project has adopted the [Microsoft Open Source Code of
Conduct](https://opensource.microsoft.com/codeofconduct/).

For more information see the [Code of Conduct
FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or contact
[opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional
questions or comments.

## Per Platform Libraries

MsPlatformDevicesLib, DfciDeviceIdSupportLib, PlatformThemeLib.
These three libraries need to be implemented per platform. An example can be found in the [NXP iMX8 platform](https://github.com/ms-iot/MU_PLATFORM_NXP).

## Issues

Please open any issues in the Project Mu GitHub tracker. [More
Details](https://microsoft.github.io/mu/How/contributing/)

## Contributing Code or Docs

Please follow the general Project Mu Pull Request process.  [More
Details](https://microsoft.github.io/mu/How/contributing/)

* [Code Requirements](https://microsoft.github.io/mu/CodeDevelopment/requirements/)
* [Doc Requirements](https://microsoft.github.io/mu/DeveloperDocs/requirements/)

## Builds

Please follow the steps in the Project Mu docs to build for CI and local
testing. [More Details](https://microsoft.github.io/mu/CodeDevelopment/compile/)

## Copyright & License

Copyright (C) Microsoft Corporation  
SPDX-License-Identifier: BSD-2-Clause-Patent

