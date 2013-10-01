#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-debug.mk)" "nbproject/Makefile-local-debug.mk"
include nbproject/Makefile-local-debug.mk
endif
endif

# Environment
MKDIR=mkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=debug
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/pic24-bootloader-firmware.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/pic24-bootloader-firmware.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=Memory.c main.c BootLoader.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/Memory.o ${OBJECTDIR}/main.o ${OBJECTDIR}/BootLoader.o
POSSIBLE_DEPFILES=${OBJECTDIR}/Memory.o.d ${OBJECTDIR}/main.o.d ${OBJECTDIR}/BootLoader.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/Memory.o ${OBJECTDIR}/main.o ${OBJECTDIR}/BootLoader.o

# Source Files
SOURCEFILES=Memory.c main.c BootLoader.c


CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-debug.mk dist/${CND_CONF}/${IMAGE_TYPE}/pic24-bootloader-firmware.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=24FJ256GB206
MP_LINKER_FILE_OPTION=,--script=p24FJ256GB206.gld
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/Memory.o: Memory.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR} 
	@${RM} ${OBJECTDIR}/Memory.o.d 
	@${RM} ${OBJECTDIR}/Memory.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Memory.c  -o ${OBJECTDIR}/Memory.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Memory.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -omf=elf -mlarge-code -mlarge-data -mconst-in-code -menable-large-arrays -O0 -D__DEBUG -msmart-io=1 -Wall -msfr-warn=off
	@${FIXDEPS} "${OBJECTDIR}/Memory.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/main.o: main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR} 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/main.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -omf=elf -mlarge-code -mlarge-data -mconst-in-code -menable-large-arrays -O0 -D__DEBUG -msmart-io=1 -Wall -msfr-warn=off
	@${FIXDEPS} "${OBJECTDIR}/main.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/BootLoader.o: BootLoader.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR} 
	@${RM} ${OBJECTDIR}/BootLoader.o.d 
	@${RM} ${OBJECTDIR}/BootLoader.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  BootLoader.c  -o ${OBJECTDIR}/BootLoader.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/BootLoader.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -omf=elf -mlarge-code -mlarge-data -mconst-in-code -menable-large-arrays -O0 -D__DEBUG -msmart-io=1 -Wall -msfr-warn=off
	@${FIXDEPS} "${OBJECTDIR}/BootLoader.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
else
${OBJECTDIR}/Memory.o: Memory.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR} 
	@${RM} ${OBJECTDIR}/Memory.o.d 
	@${RM} ${OBJECTDIR}/Memory.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Memory.c  -o ${OBJECTDIR}/Memory.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Memory.o.d"      -g -omf=elf -mlarge-code -mlarge-data -mconst-in-code -menable-large-arrays -O0 -D__DEBUG -msmart-io=1 -Wall -msfr-warn=off
	@${FIXDEPS} "${OBJECTDIR}/Memory.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/main.o: main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR} 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/main.o.d"      -g -omf=elf -mlarge-code -mlarge-data -mconst-in-code -menable-large-arrays -O0 -D__DEBUG -msmart-io=1 -Wall -msfr-warn=off
	@${FIXDEPS} "${OBJECTDIR}/main.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/BootLoader.o: BootLoader.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR} 
	@${RM} ${OBJECTDIR}/BootLoader.o.d 
	@${RM} ${OBJECTDIR}/BootLoader.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  BootLoader.c  -o ${OBJECTDIR}/BootLoader.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/BootLoader.o.d"      -g -omf=elf -mlarge-code -mlarge-data -mconst-in-code -menable-large-arrays -O0 -D__DEBUG -msmart-io=1 -Wall -msfr-warn=off
	@${FIXDEPS} "${OBJECTDIR}/BootLoader.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemblePreproc
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/pic24-bootloader-firmware.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o dist/${CND_CONF}/${IMAGE_TYPE}/pic24-bootloader-firmware.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -omf=elf -Wl,--defsym=__MPLAB_BUILD=1,--defsym=__ICD2RAM=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_ICD3=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem$(MP_EXTRA_LD_POST) 
	
else
dist/${CND_CONF}/${IMAGE_TYPE}/pic24-bootloader-firmware.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o dist/${CND_CONF}/${IMAGE_TYPE}/pic24-bootloader-firmware.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -omf=elf -Wl,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem$(MP_EXTRA_LD_POST) 
	${MP_CC_DIR}/xc16-bin2hex dist/${CND_CONF}/${IMAGE_TYPE}/pic24-bootloader-firmware.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} -a  -omf=elf 
	
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/debug
	${RM} -r dist/debug

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell "${PATH_TO_IDE_BIN}"mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
