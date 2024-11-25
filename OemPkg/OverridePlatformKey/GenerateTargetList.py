from edk2toollib.utility_functions import export_c_type_array

from edk2toollib.uefi.authenticated_variables_structure_support import (
    EfiSignatureDataFactory,
    EfiSignatureList,
)

from tempfile import TemporaryFile
from typing import Union

import uuid
import base64
import hashlib

possible_targets = [
    "Certs/PlatformKey.der"
]

replacement_cert = ("Certs/WindowsOEMDevicesPK.der",
                    "77fa9abd-0359-4d32-bd60-28f4e78f784b")


def _is_pem_encoded(certificate_data: Union[str, bytes]) -> bool:
    """This function is used to check if a certificate is pem encoded (base64 encoded).

    Args:
        certificate_data (str | bytes): The certificate to check.

    Returns:
        bool: True if the certificate is pem encoded, False otherwise.
    """
    try:
        if isinstance(certificate_data, str):
            # If there's any unicode here, an exception will be thrown and the function will return false
            sb_bytes = bytes(certificate_data, "ascii")
        elif isinstance(certificate_data, bytes):
            sb_bytes = certificate_data
        else:
            raise ValueError("Argument must be string or bytes")

        return base64.b64encode(base64.b64decode(sb_bytes)) == sb_bytes
    except Exception:
        return False


def _convert_pem_to_der(certificate_data: Union[str, bytes]) -> bytes:
    """This function is used to convert a pem encoded certificate to a der encoded certificate.

    Args:
        certificate_data: The certificate to convert.

    Returns:
        bytes: The der encoded certificate.
    """
    if isinstance(certificate_data, str):
        # If there's any unicode here, an exception will be thrown and the function will return false
        certificate_data = bytes(certificate_data, "ascii")

    return base64.b64decode(certificate_data)


def _convert_crt_to_signature_list(file: str, signature_owner: str, **kwargs: any) -> bytes:
    """This function converts a single crt file to a signature list.

    Args:
        file: The path to the crt file
        signature_owner: The signature owner. Defaults to DEFAULT_MS_SIGNATURE_GUID.

    Optional Args:
        **kwargs: Additional arguments to be passed to the function (These will be intentionally ignored)

    Returns:
        bytes: The signature list
    """
    if signature_owner is not None and not isinstance(signature_owner, uuid.UUID):
        signature_owner = uuid.UUID(signature_owner)

    siglist = EfiSignatureList(
        typeguid=EfiSignatureDataFactory.EFI_CERT_X509_GUID)

    with open(file, "rb") as crt_file, TemporaryFile() as temp_file:
        certificate = crt_file.read()
        if _is_pem_encoded(certificate):
            certificate = _convert_pem_to_der(certificate)

        temp_file.write(certificate)
        temp_file.seek(0)

        sigdata = EfiSignatureDataFactory.create(
            EfiSignatureDataFactory.EFI_CERT_X509_GUID, temp_file, signature_owner)

        # X.509 certificates are variable size, so they must be contained in their own signature list
        siglist.AddSignatureHeader(None, SigSize=sigdata.get_total_size())
        siglist.AddSignatureData(sigdata)

    return siglist.encode()


def generate_header_file(possible_targets, replacement_cert):
    """
    Generates a C header file named 'TargetList.h' that contains potential target
    SHA256 hashes and a new platform key signature list.
    The function performs the following steps:
    1. Iterates over a list of possible targets, converting each target's certificate
       to an EFI_SIGNATURE_LIST and appending it to a list.
    2. Converts a replacement certificate to an EFI_SIGNATURE_LIST.
    3. Opens (or creates) 'TargetList.h' for writing.
    4. Writes the necessary header guards and includes to the file.
    5. Iterates over the possible targets again, converting each to a SHA256 hash,
       and writes these hashes to the header file in a specific format.
    6. Writes the new platform key signature list to a temporary file and exports it
       as a C type array to the header file.
    7. Closes the header file.
    """


    # Convert the cert to an EFI_SIGNATURE_LIST
    new_platform_key_sig_list = _convert_crt_to_signature_list(
        file=replacement_cert[0], signature_owner=replacement_cert[1])

    with open("PlatformKey.h", 'w') as header_file:
        header_file.write("#ifndef PLATFORM_KEY_H_\n")
        header_file.write("#define PLATFORM_KEY_H_\n\n")

        header_file.write("#include <Uefi.h>\n")
        header_file.write("#include <Library/BaseCryptLib.h>\n")

        targets = []

        for target in possible_targets:
            # Convert the cert to a signature list
            with open(target, 'rb') as f:

                # Calculate the SHA256 hash of the signature list
                sha256_hash = hashlib.sha256()
                sha256_hash.update(f.read())
                targets.append((sha256_hash.hexdigest(), target))

        header_file.write("\n//\n")
        header_file.write("//Number of Hashes to check against\n")
        header_file.write("//\n")
        header_file.write(f"#define POTENTIAL_TARGETS {len(targets)}\n\n")

        header_file.write("\n//\n")
        header_file.write(
            "// List of certificate hashes that should be targeted\n")
        header_file.write("//\n")
        header_file.write(
            f"UINT8 PotentialTargetsHashes[POTENTIAL_TARGETS][SHA256_DIGEST_SIZE] = {{\n")
        for target in targets:
            byte_array = bytes.fromhex(target[0])
            first_half = ', '.join(f'0x{byte_array[i]:02x}' for i in range(16))
            first_half += ','
            second_half = ', '.join(
                f'0x{byte_array[i]:02x}' for i in range(16, 32))

            header_file.write(
                f"    {{ // SHA256: {target[0]} ({target[1]})\n")
            header_file.write(f"        {first_half}\n")
            header_file.write(f"        {second_half}\n")
            header_file.write("    },\n")
        header_file.write("};\n\n")

        header_file.write("//\n")
        header_file.write(
            "// In production this should be PkDefault via either SecureBootFetchData(..) or FixedPcdGetPtr (PcdDefaultPk)\n")
        header_file.write(f"// Certificate: {replacement_cert[0]}\n")
        header_file.write("//\n")

        with TemporaryFile() as new_platform_key:
            new_platform_key.write(new_platform_key_sig_list)
            new_platform_key.seek(0)

            export_c_type_array(
                new_platform_key, "NewPlatformKey", header_file)

        header_file.write("#endif // PLATFORM_KEY_H_\n")


generate_header_file(possible_targets, replacement_cert)
