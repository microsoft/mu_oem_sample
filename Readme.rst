================================
Project Mu Oem Sample Repository
================================

============================= ================= =============== ===================
 Host Type & Toolchain        Build Status      Test Status     Code Coverage
============================= ================= =============== ===================
Windows_VS_                   |WindowsCiBuild|  |WindowsCiTest| |WindowsCiCoverage|
Ubuntu_GCC5_                  |UbuntuCiBuild|   |UbuntuCiTest|  |UbuntuCiCoverage|
============================= ================= =============== ===================

This repository is part of Project Mu.  Please see Project Mu for details https://microsoft.github.io/mu

Branch Status - release/202502
==============================

:Status:git
  In Development

:Entered Development:
  2025/02/21 (Date Edk2 started accepting changes which were not in a previous release)


:Anticipated Stabilization:
  May 2025


Breaking Changes-dev
--------------------


Main Changes-dev
----------------
- OemPkg: OemPkg.dsc Removed StackCheckLib instance since it should be included via MdeLibs.dsc.inc


Code of Conduct
===============

This project has adopted the Microsoft Open Source Code of Conduct https://opensource.microsoft.com/codeofconduct/

For more information see the Code of Conduct FAQ https://opensource.microsoft.com/codeofconduct/faq/
or contact `opencode@microsoft.com <mailto:opencode@microsoft.com>`_. with any additional questions or comments.

Contributions
=============

Contributions are always welcome and encouraged!
Please open any issues in the Project Mu GitHub tracker and read https://microsoft.github.io/mu/How/contributing/


Copyright & License
===================

| Copyright (C) Microsoft Corporation
| SPDX-License-Identifier: BSD-2-Clause-Patent

.. ===================================================================
.. This is a bunch of directives to make the README file more readable
.. ===================================================================

.. CoreCI

.. _Windows_VS: https://dev.azure.com/projectmu/mu/_build/latest?definitionId=47&&branchName=release%2F202502
.. |WindowsCiBuild| image:: https://dev.azure.com/projectmu/mu/_apis/build/status/CI/OEM%20Sample/Mu%20OEM%20Sample%20-%20CI%20-%20Windows%20VS?repoName=microsoft%2Fmu_oem_sample&branchName=release%2F202502
.. |WindowsCiTest| image:: https://img.shields.io/azure-devops/tests/projectmu/mu/47.svg
.. |WindowsCiCoverage| image:: https://img.shields.io/badge/coverage-coming_soon-blue

.. _Ubuntu_GCC5: https://dev.azure.com/projectmu/mu/_build/latest?definitionId=48&&branchName=release%2F202502
.. |UbuntuCiBuild| image:: https://dev.azure.com/projectmu/mu/_apis/build/status/CI/OEM%20Sample/Mu%20OEM%20Sample%20-%20CI%20-%20GCC5?repoName=microsoft%2Fmu_oem_sample&branchName=release%2F202502
.. |UbuntuCiTest| image:: https://img.shields.io/azure-devops/tests/projectmu/mu/48.svg
.. |UbuntuCiCoverage| image:: https://img.shields.io/badge/coverage-coming_soon-blue

.. |build_status_windows| image:: https://img.shields.io/badge/build-coming_soon-red
