# APC_Injection
Inject a DLL into a specified process using APCQueueUser userland API function.  
This method of injection is good, the only issue is the fact that the thread needs to be in an alterable state.  
It also causes a program to become unresponsive in certain cases.  
You could also inject shellcode instead of a DLL, which would be better since you do not have to save anything to disk.    
# Detection
This method is detected by EDR using userland hooking.    
# Usage
1.Change the 'EXECUTABLE' macro to the process's name. Default: notepad.exe  
2.APC_Inject.exe <dll>
