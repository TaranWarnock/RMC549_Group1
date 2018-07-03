from Common.FSW_Common import *
from Command_and_Control.command_and_control import *
from Logger.logger import *
from Serial_Communication.serial_communication import *
from System_Control.system_control import *
from Telemetry.telemetry import *

if __name__ == "__main__":
    """
    This is the main function of the RMC 549 balloon(s). It spawns the threads for operation and waits until death.
    
    Writen by Daniel Letros, 2018-06-27
    """
    # Create threads
    Logging_Thread              = Logger()
    Serial_Communication_Thread = SerialCommunication(Logging_Thread)
    System_Control_Thread       = SystemControl(Logging_Thread)
    Telemetry_Thread            = Telemetry(Logging_Thread)
    Command_And_Control_Thread  = CommandAndControl(Logging_Thread,
                                                    Serial_Communication_Thread,
                                                    Telemetry_Thread,
                                                    System_Control_Thread)

    # Start threads
    Logging_Thread.start()
    Serial_Communication_Thread.start()
    System_Control_Thread.start()
    Telemetry_Thread.start()
    Command_And_Control_Thread.start()

    while True:
        # Live forever for now
        time.sleep(10000)

    # End Threads
    Command_And_Control_Thread.should_thread_run  = False
    Command_And_Control_Thread.join()
    Telemetry_Thread.should_thread_run            = False
    Telemetry_Thread.join()
    System_Control_Thread.should_thread_run       = False
    System_Control_Thread.join()
    if socket.gethostname() == "Rocky" or socket.gethostname() == "MajorTom":
        GPIO.cleanup()
    Serial_Communication_Thread.should_thread_run = False
    Serial_Communication_Thread.join()
    Logging_Thread.should_thread_run              = False
    Logging_Thread.join()














































    print("""            ,:/+/-
            /M/              .,-=;//;-
       .:/= ;MH/,    ,=/+%$XH@MM#@:
      -$##@+$###@H@MMM#######H:.    -/H#
 .,H@H@ X######@ -H#####@+-     -+H###@X
  .,@##H;      +XM##M/,     =%@###@X;-
X%-  :M##########$.    .:%M###@%:
M##H,   +H@@@$/-.  ,;$M###@%,          -
M####M=,,---,.-%%H####M$:          ,+@##
@##################@/.         :%H##@$-
M###############H,         ;HM##M$=
#################.    .=$M##M$=
################H..;XM##M$=          .:+
M###################@%=           =+@MH%
@################M/.          =+H#X%=
=+M##############M,       -/X#X+;.
  .;XM##########H=    ,/X#H+:,
     .=+HM######M+/+HM@+=.
         ,:/%XM####H/.
              ,.:=-.                    """)
    print("This was a triumph!")
    print("I'm making a note here:")
    print("Huge success!")
