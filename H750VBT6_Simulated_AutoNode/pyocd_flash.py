import subprocess
import sys

def flash_firmware(firmware_path, target_device):
    """
    Flash the firmware to the specified target device using PyOCD.

    :param firmware_path: Path to the firmware file.
    :param target_device: Name of the target device (e.g., STM32H750VBTx).
    """
    # Build the PyOCD command
    command = [
        "pyocd", 
        "flash", 
        "-t", 
        target_device, 
        firmware_path
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
firmware_path = "build/Simulated-GW-ECU.elf"
target_device = "STM32H750VBTx"  # Replace with your target device name
flash_firmware(firmware_path, target_device)