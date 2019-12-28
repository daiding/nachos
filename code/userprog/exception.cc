// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "machine.h"
#include "addrspace.h"
#include "processmanager.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions
//	are in machine.h.
//----------------------------------------------------------------------
static void ExceptionPageFaultHandler();
static void SysCallInitial(int arg);
static void SysCallExecHandler();
static void SysCallExitHandler();
static void SysCallJoinHandler();
//static void SysCallForkHandler();

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);
    switch (which)
	{
		case SyscallException:
			switch (type)
			{
				case SC_Halt:
					printf("this machine halt!!\n");
					interrupt->Halt();
					break;
				case SC_Exit:
					SysCallExitHandler();
					break;
				case SC_Exec:
					printf("process %d call Exec\n",currentThread->space->GetProcessID());
					SysCallExecHandler();
					break;
				case SC_Join:
					printf("process %d call Join\n",currentThread->space->GetProcessID());
					SysCallJoinHandler();
					break;
				default:
					printf("Unexpected system call type %d!\n", type);
					interrupt->Halt();
					break;
			}
			break;
		case PageFaultException:
			ExceptionPageFaultHandler();
			break;
		default:
			printf("Unexpected user mode exception %d %d\n", which, type);
			break;
	}
	return;
}

static void ExceptionPageFaultHandler()
{
    int badVirtualAddr = machine->ReadRegister(39);
    int badVirtualPageNO = badVirtualAddr / PageSize;
	DEBUG('a',"PAGE FAULT START PROCESSING\n");
    memoryManager->ProcessPageFault(badVirtualPageNO);
    return;
}

static void SysCallInitial(int arg)
{
	switch (arg)
	{
		case 0:
			DEBUG('a',"Change the page table and register of process %d", currentThread->space->GetProcessID());
			currentThread->space->RestoreState();
			currentThread->space->InitRegisters();
			break;
		case 1:
			currentThread->space->RestoreState();
			break;
		default:
			break;
	}
	machine->Run();
	return;
}

static void SysCallExecHandler()
{
	char fileName[50];
	int i = 0;
	int fileNameAddress = machine->ReadRegister(4);
	do
	{
		machine->ReadMem(fileNameAddress+i,1,(int*)(fileName+i));
	} while (i < 50 && fileName[i++]!='\0');
	OpenFile* executableFile = fileSystem->Open(fileName);
	ProcessControlBlock* processControlBlock = processManager->CreateProcess(executableFile);
	if (processControlBlock != NULL && executableFile != NULL)
	{
		Thread* mainThread = new Thread(fileName);
		processControlBlock->SetMainThread(mainThread);
		//mainThread->space = processControlBlock->GetProcessSpace();
		machine->WriteRegister(2, processControlBlock->GetProcessID());
		mainThread->Fork(SysCallInitial, 0);
		mainThread->SetParentThread(currentThread);
		currentThread->AddChildThread(mainThread);
		printf("Exec %s, new process pid is %d\n", fileName, processControlBlock->GetProcessID());
	}
	else 
	{
		printf("Exec FAIL!! Return -1！！\n");
        machine->WriteRegister(2, -1);
	}
	int cur_PC = machine->ReadRegister(PCReg);
	machine->WriteRegister(PrevPCReg, cur_PC);//通过修改寄存器，使cpu运行下一条指令
	machine->WriteRegister(PCReg, cur_PC + sizeof(int));
	machine->WriteRegister(NextPCReg, cur_PC + 2 * sizeof(int));
	return;
}

static void SysCallExitHandler()
{
	int exitStatus = machine->ReadRegister(4);
	printf("process %d call Exit, exit status is %d\n",currentThread->space->GetProcessID(), exitStatus);
	currentThread->SetExitStatus(exitStatus);
	Thread* parent = currentThread->GetParentThread();
	DEBUG('a', "Get the parent thread!\n");
	if (parent != NULL && parent->GetStatus() == BLOCKED)
	{
		scheduler->ReadyToRun(parent);
		DEBUG('a', "Wake up the join process!\n");
	}
	
	currentThread->Finish();
	return;
}

static void SysCallJoinHandler()
{
	int pid= machine->ReadRegister(4);
	DEBUG('a', "Join the child process %d\n", pid);
	Thread* child = processManager->GetMainThread(pid);
	if (child == NULL)//系统调用出错
	{
		machine->WriteRegister(2, -1);
		return;
	}
	while(!currentThread->RemoveExitedChild(child))
	{
		printf("Process %d sleep to wait the child process %d exit\n",currentThread->space->GetProcessID(),child->space->GetProcessID());
		DEBUG('a',"the process %d sleep\n", currentThread->space->GetProcessID());
		(void) interrupt->SetLevel(IntOff);
		currentThread->Sleep();
	}
	int exitStatus = child->GetExitStatus();
	delete child;
	machine->WriteRegister(2,exitStatus);
	printf("the join child process %d exit status = %d\n", pid, exitStatus);
	int cur_PC = machine->ReadRegister(PCReg);
	machine->WriteRegister(PrevPCReg, cur_PC);//通过修改寄存器，使cpu运行下一条指令
	machine->WriteRegister(PCReg, cur_PC + sizeof(int));
	machine->WriteRegister(NextPCReg, cur_PC + 2 * sizeof(int));
	return;
}
