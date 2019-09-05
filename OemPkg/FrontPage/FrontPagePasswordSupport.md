# FrontPage Password Support

## Copyright

Copyright (C) Microsoft Corporation. All rights reserved.
SPDX-License-Identifier: BSD-2-Clause-Patent

## About

OemPkg FrontPage application is a sample UEFI UI app that among other features supports password authentication.

## Password Architecture

Below architecture diagram shows how the password support is integrated into the FrontPage application.
![Architecture Diagram](PasswordArchitecture_mu.png "Architecture Diagram")

## Setting a Password

FrontPage links to PasswordPolicyLib in order to validate the user-provided password and then to create a hash out of the password. Then, the Settings Access Protocol is used to save the hash. DfciPasswordProvider is linked to DfciPkg SettingsManagerDxe to register the password setting with the Settings Access Protocol. DfciPasswordProvider links to PasswordStoreLib to set the password via PasswordStoreSetPassword.

## Authenticating a Password

FrontPage uses the DFCI Authentication Protocol to authenticate a password via the AuthWithPW interface and acquire an authentication token. DFCI Authentication Protocol is installed by DFCI Identity and Auth Manager, which uses PasswordStoreLib to check whether a password is set, and if set, then to authenticate the password.
