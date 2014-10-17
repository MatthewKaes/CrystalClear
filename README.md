Crystal Clear
=============

Dynamic languages tend to be slow. This is a product of a number of factors associated with dynamic languages. 

1. Most high level languages are not compiled natively and are instead compiled down to bytecode which is then interpreted.
2. Abstract memory management away from the user to make it simpler and less error prone. This adds the need for a garbage collector under the hood to manage all of the memory automatically.
3. Resolving types at runtime which adds unnecessary type checking overhead. 

These are just some of the factors that make dynamic languages.

##Performance First Language

Crystal Clear solves these problems by breaking it down into two responsibilities: Crystal responsibiliites and Clear responsibilities. Crystal is responsable for compileing code and managing memory  to tackle points 1 and 2. Clear is responsible for resolving the minimum control paths required to execute dynamic code. By doing this Crystal Clear aims to be a performance first language doing the best it can to preform operations as fast as possible. In order to bridge the performance gap between statically compiled languages like C/C++ and traditional high level dynamic languages a number of methods are used.

##The "Crystal"

###[Ahead of Time Compiler](https://github.com/MatthewKaes/DynamicCompiler)

The key feature of Crystal is its use of an AOT (Ahead of Time) Compiler to turn crystal code directly into native machine code to remove the overhead of bytecode. Rather than jumping in and out of bytecode, Crystal executes the operations directly similar to how C/C++ work. This also allows for low level optimizations in how the machine code is created giving instruction level control.

###Resource Acquisition Is Initialization

Variables have function scooping rather than standard scooping techniques making it so variables are easy to construct and destruct. Rather than standard garbage collection variables are compiled to know their lifetime using clarity filters so they are only cleaned up when dynamic data reaches it's lifetime. Dynamic data is always cleaned up only when necessary to minimize the footprint of memory management on the program.

##The "Clear"

###Clarity Filters

Clarity Filters are the main part of what enables Crystal to run as fast as possible. Clarity Filters are used to keep track of all possible states a variable or symbol can take. This allows the compiler to only produce the minimum amount of machine code to resolve any symbol operation at runtime. In some cases this even allows for static compiling when symbols only ever take on a single type at any time. 

An example snippit of how clarity filters work under the hood can be found on my blog [here](http://crystalclearprogramming.blogspot.com/2014/06/clarity-im-going-insane.html).

###Clear Hooks

Sometimes the code you are looking to write already exist or you want more control over the machine code that is produced. In these cases you can call out to C/C++ code compiled directly into the compiler or call functions from DLL files. This gives the user the ability to easily get Crystal talking to or invoking other languages.

Crystal also has hooks for Python using Boost.Python. If a version of python is installed and the python libraries are built for boost then scripts can be run directly from Crystal. 


##Useable and Fast

Crystal is built in the hopes to merge the abilities of high level languages like Ruby without making any sacrifices in speed. In the end Crystal uses a combination of features to put performance first and usability in a close second.

##Requirements

Crystal Clear requires [Boost 1.56 or newer to build](http://sourceforge.net/projects/boost/files/boost/1.56.0/). If you choose to use the visual studio 2010 solution then make sure to set up boost in C:\Program Files\boost\boost_1_56_0\. You can find out more about building boost [here](http://www.boost.org/doc/libs/1_56_0/more/getting_started/windows.html). Crystal Clear currently has several dependencies on Windows but in the future Crystal will be made to run on any operating system. The only machine currently implemented for Crystal is the x86 machine. Other Archetectures are not supported at this time.

For a quick setup using MSVC 10 (Visual studios 2010) you can build boost with the following command in a command window in the boost directory:
b2 toolset=msvc-10.0 link=static threading=multi variant=release,debug runtime-link=static

##Updates on development

Updates on the development of Crystal Clear can currently be found on my blog [Crystal Clear Programming](http://crystalclearprogramming.blogspot.com/search/label/Crystal%20Clear).
