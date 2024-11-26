import subprocess
import sys

def flash_firmware(firmware_path, interface_config, target_config):
    """
    Flash the firmware to the specified target device using OpenOCD.

    :param firmware_path: Path to the firmware file.
    :param interface_config: Path to the OpenOCD interface configuration file.
    :param target_config: Path to the OpenOCD target configuration file.
    """
    # Build the OpenOCD command
    command = [
        "openocd",
        "-f", interface_config,
        "-f", target_config,
        "-c", f"program {firmware_path}  verify reset exit 0x08000000"
    ]
    
    # Execute the command and implement real-time output
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    
    # Read and print standard output in real-time
    for line in process.stdout:
        print(line.strip())
    
    # Read and print standard error in real-time
    for line in process.stderr:
        print(line.strip(), file=sys.stderr)
    
    # Wait for the process to finish and get the return code
    return_code = process.wait()
    
    if return_code == 0:
        print("Firmware flashing successful")
    else:
        print("Firmware flashing failed")

# Example usage
firmware_path = "build/Simulated-GW-ECU.bin"
interface_config = "openocd-scripts/interface/cmsis-dap.cfg"
target_config = "openocd-scripts/target/stm32h7x.cfg"
flash_firmware(firmware_path, interface_config, target_config)