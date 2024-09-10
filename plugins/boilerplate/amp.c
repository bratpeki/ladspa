/*
 * This is a modified variant of the original example amplifier.
 *
 * This is meant to be easily compiled, and doesn't use multiple C source or header files.
 * It also utilizes just one, bare-bones Makefile that compiles using GCC.
 *
 * The variable naming conventions used are:
 *   Structs          | s...    |
 *   Ports            | p...    |
 *   Data             | d...    | This is any LADSPA_Data used by the plugin
 *   Descriptors      | desc... | descm for moro, descs for stereo
 *   Port descriptors | pd...   |
 *   Port range hints | prh...  |
 */

#include <stdlib.h>
#include <string.h>

#include <ladspa.h>

#ifndef __cplusplus
# if __GNUC__
#  define ON_LOAD_ROUTINE   static void __attribute__ ((constructor)) init()
#  define ON_UNLOAD_ROUTINE static void __attribute__ ((destructor))  fini()
# else
#  define ON_LOAD_ROUTINE   void _init()
#  define ON_UNLOAD_ROUTINE void _fini()
# endif
#else
#endif

#define AMP_CONTROL 0
#define AMP_INPUT1 1
#define AMP_OUTPUT1 2
#define AMP_INPUT2 3
#define AMP_OUTPUT2 4

typedef struct {

	LADSPA_Data* pControl;
	LADSPA_Data* pInput1;
	LADSPA_Data* pOutput1;
	LADSPA_Data* pInput2;  /* Stereo */
	LADSPA_Data* pOutput2; /* Stereo */

} Amplifier;

static LADSPA_Handle instantiateAmplifier(
	const LADSPA_Descriptor* Descriptor,
	unsigned long SampleRate
) {
	return malloc(sizeof(Amplifier));
}

static void connectPortToAmplifier(
	LADSPA_Handle Instance,
	unsigned long Port,
	LADSPA_Data* DataLocation
) {

	Amplifier* sAmplifier;

	sAmplifier = (Amplifier*)Instance;

	switch (Port) {

		case AMP_CONTROL: sAmplifier->pControl = DataLocation; break;
		case AMP_INPUT1:  sAmplifier->pInput1  = DataLocation; break;
		case AMP_OUTPUT1: sAmplifier->pOutput1 = DataLocation; break;
		case AMP_INPUT2:  sAmplifier->pInput2  = DataLocation; break; /* Stereo */
		case AMP_OUTPUT2: sAmplifier->pOutput2 = DataLocation; break; /* Stereo */

	}

}

static void runMonoAmplifier(
	LADSPA_Handle Instance,
	 unsigned long SampleCount
) {

	LADSPA_Data* dInput;
	LADSPA_Data* dOutput;
	LADSPA_Data dGain;
	Amplifier* sAmplifier;
	unsigned long sampleIndex;

	sAmplifier = (Amplifier*)Instance;

	dInput = sAmplifier->pInput1;
	dOutput = sAmplifier->pOutput1;
	dGain = *(sAmplifier->pControl);

	for (sampleIndex = 0; sampleIndex < SampleCount; sampleIndex++)
		*(dOutput++) = *(dInput++)* dGain;

}

static void runStereoAmplifier(
	LADSPA_Handle Instance,
	unsigned long SampleCount
) {

	LADSPA_Data* dInput;
	LADSPA_Data* dOutput;
	LADSPA_Data dGain;
	Amplifier* sAmplifier;
	unsigned long sampleIndex;

	sAmplifier = (Amplifier*)Instance;

	dGain = *(sAmplifier->pControl);

	dInput = sAmplifier->pInput1;
	dOutput = sAmplifier->pOutput1;
	for (sampleIndex = 0; sampleIndex < SampleCount; sampleIndex++)
		*(dOutput++) = *(dInput++)* dGain;

	dInput = sAmplifier->pInput2;
	dOutput = sAmplifier->pOutput2;
	for (sampleIndex = 0; sampleIndex < SampleCount; sampleIndex++)
		*(dOutput++) = *(dInput++)* dGain;

}

static void cleanupAmplifier( LADSPA_Handle Instance ) { free(Instance); }

LADSPA_Descriptor* descmAmplifier = NULL;
LADSPA_Descriptor* descsAmplifier = NULL;

ON_LOAD_ROUTINE {

	char ** pcPortNames;
	LADSPA_PortDescriptor* pdAmplifier;
	LADSPA_PortRangeHint* prhAmplifier;

	descmAmplifier = (LADSPA_Descriptor*)malloc(sizeof(LADSPA_Descriptor));
	descsAmplifier = (LADSPA_Descriptor*)malloc(sizeof(LADSPA_Descriptor));

	if (descmAmplifier) {

		descmAmplifier->UniqueID = 1;

		descmAmplifier->Label = strdup("peki_amp_mono");
		descmAmplifier->Name = strdup("Peki's Mono Amplifier");
		descmAmplifier->Maker = strdup("Petar Katić (bratpeki)");
		descmAmplifier->Copyright = strdup("None");

		descmAmplifier->PortCount = 3;
		descmAmplifier->Properties = LADSPA_PROPERTY_HARD_RT_CAPABLE;

		pdAmplifier = (LADSPA_PortDescriptor*)calloc(3, sizeof(LADSPA_PortDescriptor));
		pdAmplifier[AMP_CONTROL] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
		pdAmplifier[AMP_INPUT1] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
		pdAmplifier[AMP_OUTPUT1] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
		descmAmplifier->PortDescriptors = (const LADSPA_PortDescriptor*)pdAmplifier;

		pcPortNames = (char **)calloc(3, sizeof(char*));
		pcPortNames[AMP_CONTROL] = strdup("Gain");
		pcPortNames[AMP_INPUT1] = strdup("Input");
		pcPortNames[AMP_OUTPUT1] = strdup("Output");
		descmAmplifier->PortNames = (const char **)pcPortNames;

		prhAmplifier = ((LADSPA_PortRangeHint*)calloc(3, sizeof(LADSPA_PortRangeHint)));
		prhAmplifier[AMP_CONTROL].HintDescriptor = (
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_LOGARITHMIC | LADSPA_HINT_DEFAULT_1
		);
		prhAmplifier[AMP_CONTROL].LowerBound = 0;
		prhAmplifier[AMP_INPUT1].HintDescriptor = 0;
		prhAmplifier[AMP_OUTPUT1].HintDescriptor = 0;
		descmAmplifier->PortRangeHints = (const LADSPA_PortRangeHint*)prhAmplifier;

		descmAmplifier->instantiate = instantiateAmplifier;
		descmAmplifier->connect_port = connectPortToAmplifier;
		descmAmplifier->activate = NULL;
		descmAmplifier->run = runMonoAmplifier;
		descmAmplifier->run_adding = NULL;
		descmAmplifier->set_run_adding_gain = NULL;
		descmAmplifier->deactivate = NULL;
		descmAmplifier->cleanup = cleanupAmplifier;
	}

	if (descsAmplifier) {

		descsAmplifier->UniqueID = 2;

		descsAmplifier->Label = strdup("peki_amp_stereo");
		descsAmplifier->Name = strdup("Peki's Stereo Amplifier");
		descsAmplifier->Maker = strdup("Petar Katić (bratpeki)");
		descsAmplifier->Copyright = strdup("None");

		descsAmplifier->PortCount = 5;
		descsAmplifier->Properties = LADSPA_PROPERTY_HARD_RT_CAPABLE;

		pdAmplifier = (LADSPA_PortDescriptor*)calloc(5, sizeof(LADSPA_PortDescriptor));
		pdAmplifier[AMP_CONTROL] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
		pdAmplifier[AMP_INPUT1] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
		pdAmplifier[AMP_OUTPUT1] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
		pdAmplifier[AMP_INPUT2] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
		pdAmplifier[AMP_OUTPUT2] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
		descsAmplifier->PortDescriptors = (const LADSPA_PortDescriptor*)pdAmplifier;

		pcPortNames = (char **)calloc(5, sizeof(char*));
		pcPortNames[AMP_CONTROL] = strdup("Gain");
		pcPortNames[AMP_INPUT1] = strdup("Input (Left)");
		pcPortNames[AMP_OUTPUT1] = strdup("Output (Left)");
		pcPortNames[AMP_INPUT2] = strdup("Input (Right)");
		pcPortNames[AMP_OUTPUT2] = strdup("Output (Right)");
		descsAmplifier->PortNames = (const char **)pcPortNames;

		prhAmplifier = ((LADSPA_PortRangeHint*)calloc(5, sizeof(LADSPA_PortRangeHint)));
		prhAmplifier[AMP_CONTROL].HintDescriptor = (
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_LOGARITHMIC | LADSPA_HINT_DEFAULT_1
		);
		prhAmplifier[AMP_CONTROL].LowerBound = 0;
		prhAmplifier[AMP_INPUT1].HintDescriptor = 0;
		prhAmplifier[AMP_OUTPUT1].HintDescriptor = 0;
		prhAmplifier[AMP_INPUT2].HintDescriptor = 0;
		prhAmplifier[AMP_OUTPUT2].HintDescriptor = 0;
		descsAmplifier->PortRangeHints = (const LADSPA_PortRangeHint*)prhAmplifier;

		descsAmplifier->instantiate = instantiateAmplifier;
		descsAmplifier->connect_port = connectPortToAmplifier;
		descsAmplifier->activate = NULL;
		descsAmplifier->run = runStereoAmplifier;
		descsAmplifier->run_adding = NULL;
		descsAmplifier->set_run_adding_gain = NULL;
		descsAmplifier->deactivate = NULL;
		descsAmplifier->cleanup = cleanupAmplifier;

	}

}

static void deleteDescriptor(LADSPA_Descriptor* psDescriptor) {

	unsigned long lIndex;

	if (psDescriptor) {
		free((char*)psDescriptor->Label);
		free((char*)psDescriptor->Name);
		free((char*)psDescriptor->Maker);
		free((char*)psDescriptor->Copyright);
		free((LADSPA_PortDescriptor*)psDescriptor->PortDescriptors);
		for (lIndex = 0; lIndex < psDescriptor->PortCount; lIndex++)
			free((char*)(psDescriptor->PortNames[lIndex]));
		free((char**)psDescriptor->PortNames);
		free((LADSPA_PortRangeHint*)psDescriptor->PortRangeHints);
		free(psDescriptor);
	}

}

ON_UNLOAD_ROUTINE {
	deleteDescriptor(descmAmplifier);
	deleteDescriptor(descsAmplifier);
}

const LADSPA_Descriptor* ladspa_descriptor(unsigned long Index) {

	switch (Index) {
		case 0: return descmAmplifier;
		case 1: return descsAmplifier;
		default: return NULL;
	}

}
