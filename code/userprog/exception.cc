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
				case SC_Exec:
					SysCallExecHandler();
					break;
				default:
					printf("Unexpected system call type !\n");
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
    int badVisualAddr = machine->ReadRegister(39);
    int badVisualPageNO = badVisualAddr / PageSize;
	printf("PAGE FAULT START PROCESSING\n");
    memoryManager->ProcessPageFault(badVisualPageNO);
    return;
}

static void SysCallInitial(int arg)
{
	switch (arg)
	{
		case 0:
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
		fileName[i] = machine->ReadMem(fileNameAddress+i,1,fileName+i);
	} while (i < 50&&fileName[i++]!='\0');
	OpenFile* executableFile = fileSystem->Open(fileName);
	AddrSpace* processAddrSpace = processManager->CreateAddrSpace(executableFile);
	Thread* mainThread = new Thread(fileName);
	if (processAddrSpace != NULL && executableFile != NULL)
	{
		Thread* mainThread = new Thread(fileName);
		mainThread->space = processAddrSpace;
		machine->WriteRegister(2, processAddrSpace->GetProcessID());
		mainThread->Fork(SysCallInitial, 0);
	}
	else 
	{
		machine->WriteRegister(2, -1);
	}
	machine->PCForword();
	return;
}