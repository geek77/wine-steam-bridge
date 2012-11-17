#!/usr/bin/env python
import re

frx = re.compile(r'\([^)]*\)')

def read_spec_file(f):
    result = []    
    for _x in f.readlines():
	x = re.sub(frx,"",_x)
	try:
	    i,t,name = x.split(" ")
	    result.append(name[:-1])
	except:
	    continue
    return result

def call_trace(i):
    return """\"subl $8,%%esp\\n"
    "movl $%i,%%eax\\n"
    "movl %%eax, 4(%%esp)\\n"
    "movl $trace_format, (%%esp)\\n"
    "call printf\n"
    "addl $8,%%esp\n" """

    
    
    
if __name__=="__main__":
    functions = read_spec_file(open("steam_api.spec"))
    names = ""
    addresses = ""
    for x in functions:
	names = names + "\n\t\t\"%s\"," % x
	addresses = addresses + "\n\t\tSTUB /* %s */," % x

    print """
    char *name_table[%i] = { %s 
    };
    
    """ % (len(functions),names[:-1])
    
    print """
    void *addr_table[%i] = { %s
    };""" % (len(functions),addresses[:-1])
    
    print """
    char trace_format[] = "%s\\n";
    """
#    void do_not_call(int c) {
#	if(c!=9) return;
#    """
    i = 0;
    for x in functions:
	print """
	asm(".globl %s\\n"
	"%s:\\n"	
	"movl $%i,%%eax\\n"
	"movl name_table(,%%eax,4),%%eax\\n"	
	"subl $8, %%esp\\n"
	"movl %%eax, 4(%%esp)\\n"
	"movl $trace_format, (%%esp)\\n"
	"call printf\\n"
	"addl $8,%%esp\\n"
	"movl $%i,%%eax\\n"
	"movl addr_table(,%%eax,4),%%eax\\n"
	"jmp *%%eax\\n");
	""" % (x,x,i,i) # : : : "%%eax" );
	i = i + 1
    
#    print "};"
