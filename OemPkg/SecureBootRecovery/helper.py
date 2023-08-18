# Take the contents of the Payload folder and convert it into a C Header file

from edk2toollib.utility_functions import export_c_type_array

with open("Payload/dbUpdate.bin", "rb") as payload_fs, open("./RecoveryPayload.h", "w") as output_fs:
    output_fs.write("#ifndef _RECOVERY_PAYLOAD_H_\n")
    output_fs.write("#define _RECOVERY_PAYLOAD_H_\n")
    output_fs.write("#include <Uefi.h>\n")


    export_c_type_array(payload_fs, "DbUpdate", output_fs)

    output_fs.write("#endif // _RECOVERY_PAYLOAD_H_\n")
