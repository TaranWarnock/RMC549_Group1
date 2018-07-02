from Common.FSW_Common import *
from Command_and_Control.command_and_control import *
from Logger.logger import *
from Serial_Communication.serial_communication import *

if __name__ == "__main__":
    """
    This is the main function of the RMC 549 balloon(s). It spawns the threads for operation and waits until death.
    
    Writen by Daniel Letros, 2018-06-27
    """
    # Create threads
    Logging_Thread              = Logger()
    Serial_Communication_Thread = SerialCommunication(Logging_Thread)
    Command_And_Control_Thread  = CommandAndControl(Logging_Thread, Serial_Communication_Thread)

    # Start threads
    Logging_Thread.start()
    Serial_Communication_Thread.start()
    Command_And_Control_Thread.start()

    while True:
        # Live forever for now
        time.sleep(10000)

    # End Threads
    Command_And_Control_Thread.should_thread_run  = False
    Command_And_Control_Thread.join()
    Serial_Communication_Thread.should_thread_run = False
    Serial_Communication_Thread.join()
    Logging_Thread.should_thread_run              = False
    Logging_Thread.join()
    print("Exiting Main Thread")


