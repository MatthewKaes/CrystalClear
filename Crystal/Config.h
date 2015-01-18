#ifndef CRYSTAL_CONFIG
#define CRYSTAL_CONFIG

//=================================
//Use this config file to configure
//your compiler features.
//=================================


//==================================
//Python
//==================================
// Requires Python 2.2 or newer
#define INCLUDE_PYTHON 0

// The maximum size of script allowed to be loaded in Crystal.
#define PYTHON_SCRIPT_SIZE 4096

//==================================
//Garbage Collection
//==================================
// Block size to be allocated for each page
// Multiply by the size of a crystal symbol to get the total size.
// Default is 16KB for a 32bit x86 machine
#define MAX_PAGE_SIZE 1024

// Max number of blocks that the deadpool will preserve
// Blocks in excess of this ammount will be deleted.
#define MAX_FREE_LIST 64

// The minimum about of time to wait between collections.
// (Time is in milliseconds)
#define COLLECTION_DELAY 150

// turns off all all default optimizations
// Equivilant of "debug" mode.
#define DEBUG_MACHINECODE true

//==================================
// Optimizations (slows startup)
//==================================
// Always on optimizations:
// (These cannot be turned off to improve start preformance)
// - Const reductions
// - Common registry optimizations
// - Regeistry optimizations
// - Return optimizations

// remove uncessecary branches
#define DEAD_CODE_ELIMINATION true

// removes redundent registry assignmetns (expensive)
#define REDUCE_MOV false

// reduce instruction wait times and pair parallel instructions
// together. Enabled in a simple way by default.
#define MACHINE_SCHEDULING false

// unrolls loops to prevent branches (very expensive)
#define LOOP_UNROOLING false

// try to optimize code by making spot by spot improvements
#define PIGEONHOLING false

#endif