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
 *   Port names       | pn...   |
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

} PekiAmp;

static LADSPA_Handle instantiatePekiAmp(
	const LADSPA_Descriptor* Descriptor,
	unsigned long SampleRate
) {
	return malloc(sizeof(PekiAmp));
}

static void connectPortToPekiAmp(
	LADSPA_Handle Instance,
	unsigned long Port,
	LADSPA_Data* DataLocation
) {

	PekiAmp* sPekiAmp;

	sPekiAmp = (PekiAmp*)Instance;

	switch (Port) {

		case AMP_CONTROL: sPekiAmp->pControl = DataLocation; break;
		case AMP_INPUT1:  sPekiAmp->pInput1  = DataLocation; break;
		case AMP_OUTPUT1: sPekiAmp->pOutput1 = DataLocation; break;
		case AMP_INPUT2:  sPekiAmp->pInput2  = DataLocation; break; /* Stereo */
		case AMP_OUTPUT2: sPekiAmp->pOutput2 = DataLocation; break; /* Stereo */

	}

}

static void runMonoPekiAmp(
	LADSPA_Handle Instance,
	 unsigned long SampleCount
) {

	LADSPA_Data* dInput;
	LADSPA_Data* dOutput;
	LADSPA_Data dGain;
	PekiAmp* sPekiAmp;
	unsigned long sampleIndex;

	sPekiAmp = (PekiAmp*)Instance;

	dInput = sPekiAmp->pInput1;
	dOutput = sPekiAmp->pOutput1;
	dGain = *(sPekiAmp->pControl);

	for (sampleIndex = 0; sampleIndex < SampleCount; sampleIndex++)
		*(dOutput++) = *(dInput++)* dGain;

}

static void runStereoPekiAmp(
	LADSPA_Handle Instance,
	unsigned long SampleCount
) {

	LADSPA_Data* dInput;
	LADSPA_Data* dOutput;
	LADSPA_Data dGain;
	PekiAmp* sPekiAmp;
	unsigned long sampleIndex;

	sPekiAmp = (PekiAmp*)Instance;

	dGain = *(sPekiAmp->pControl);

	dInput = sPekiAmp->pInput1;
	dOutput = sPekiAmp->pOutput1;
	for (sampleIndex = 0; sampleIndex < SampleCount; sampleIndex++)
		*(dOutput++) = *(dInput++)* dGain;

	dInput = sPekiAmp->pInput2;
	dOutput = sPekiAmp->pOutput2;
	for (sampleIndex = 0; sampleIndex < SampleCount; sampleIndex++)
		*(dOutput++) = *(dInput++)* dGain;

}

static void cleanupPekiAmp( LADSPA_Handle Instance ) { free(Instance); }

LADSPA_Descriptor* descmPekiAmp = NULL;
LADSPA_Descriptor* descsPekiAmp = NULL;

ON_LOAD_ROUTINE {

	char ** pnPekiAmp;
	LADSPA_PortDescriptor* pdPekiAmp;
	LADSPA_PortRangeHint* prhPekiAmp;

	descmPekiAmp = (LADSPA_Descriptor*)malloc(sizeof(LADSPA_Descriptor));
	descsPekiAmp = (LADSPA_Descriptor*)malloc(sizeof(LADSPA_Descriptor));

	if (descmPekiAmp) {

		descmPekiAmp->UniqueID = 1;

		descmPekiAmp->Label = strdup("peki_amp_mono");
		descmPekiAmp->Name = strdup("Peki's Mono PekiAmp");
		descmPekiAmp->Maker = strdup("Petar Katić (bratpeki)");
		descmPekiAmp->Copyright = strdup("None");

		descmPekiAmp->PortCount = 3;
		descmPekiAmp->Properties = LADSPA_PROPERTY_HARD_RT_CAPABLE;

		pdPekiAmp = (LADSPA_PortDescriptor*)calloc(3, sizeof(LADSPA_PortDescriptor));
		pdPekiAmp[AMP_CONTROL] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
		pdPekiAmp[AMP_INPUT1] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
		pdPekiAmp[AMP_OUTPUT1] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
		descmPekiAmp->PortDescriptors = (const LADSPA_PortDescriptor*)pdPekiAmp;

		pnPekiAmp = (char **)calloc(3, sizeof(char*));
		pnPekiAmp[AMP_CONTROL] = strdup("Gain");
		pnPekiAmp[AMP_INPUT1] = strdup("Input");
		pnPekiAmp[AMP_OUTPUT1] = strdup("Output");
		descmPekiAmp->PortNames = (const char **)pnPekiAmp;

		prhPekiAmp = ((LADSPA_PortRangeHint*)calloc(3, sizeof(LADSPA_PortRangeHint)));
		prhPekiAmp[AMP_CONTROL].HintDescriptor = (
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_LOGARITHMIC | LADSPA_HINT_DEFAULT_1
		);
		prhPekiAmp[AMP_CONTROL].LowerBound = 0;
		prhPekiAmp[AMP_INPUT1].HintDescriptor = 0;
		prhPekiAmp[AMP_OUTPUT1].HintDescriptor = 0;
		descmPekiAmp->PortRangeHints = (const LADSPA_PortRangeHint*)prhPekiAmp;

		descmPekiAmp->instantiate = instantiatePekiAmp;
		descmPekiAmp->connect_port = connectPortToPekiAmp;
		descmPekiAmp->activate = NULL;
		descmPekiAmp->run = runMonoPekiAmp;
		descmPekiAmp->run_adding = NULL;
		descmPekiAmp->set_run_adding_gain = NULL;
		descmPekiAmp->deactivate = NULL;
		descmPekiAmp->cleanup = cleanupPekiAmp;
	}

	if (descsPekiAmp) {

		descsPekiAmp->UniqueID = 2;

		descsPekiAmp->Label = strdup("peki_amp_stereo");
		descsPekiAmp->Name = strdup("Peki's Stereo PekiAmp");
		descsPekiAmp->Maker = strdup("Petar Katić (bratpeki)");
		descsPekiAmp->Copyright = strdup("None");

		descsPekiAmp->PortCount = 5;
		descsPekiAmp->Properties = LADSPA_PROPERTY_HARD_RT_CAPABLE;

		pdPekiAmp = (LADSPA_PortDescriptor*)calloc(5, sizeof(LADSPA_PortDescriptor));
		pdPekiAmp[AMP_CONTROL] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
		pdPekiAmp[AMP_INPUT1] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
		pdPekiAmp[AMP_OUTPUT1] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
		pdPekiAmp[AMP_INPUT2] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
		pdPekiAmp[AMP_OUTPUT2] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
		descsPekiAmp->PortDescriptors = (const LADSPA_PortDescriptor*)pdPekiAmp;

		pnPekiAmp = (char **)calloc(5, sizeof(char*));
		pnPekiAmp[AMP_CONTROL] = strdup("Gain");
		pnPekiAmp[AMP_INPUT1] = strdup("Input (Left)");
		pnPekiAmp[AMP_OUTPUT1] = strdup("Output (Left)");
		pnPekiAmp[AMP_INPUT2] = strdup("Input (Right)");
		pnPekiAmp[AMP_OUTPUT2] = strdup("Output (Right)");
		descsPekiAmp->PortNames = (const char **)pnPekiAmp;

		prhPekiAmp = ((LADSPA_PortRangeHint*)calloc(5, sizeof(LADSPA_PortRangeHint)));
		prhPekiAmp[AMP_CONTROL].HintDescriptor = (
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_LOGARITHMIC | LADSPA_HINT_DEFAULT_1
		);
		prhPekiAmp[AMP_CONTROL].LowerBound = 0;
		prhPekiAmp[AMP_INPUT1].HintDescriptor = 0;
		prhPekiAmp[AMP_OUTPUT1].HintDescriptor = 0;
		prhPekiAmp[AMP_INPUT2].HintDescriptor = 0;
		prhPekiAmp[AMP_OUTPUT2].HintDescriptor = 0;
		descsPekiAmp->PortRangeHints = (const LADSPA_PortRangeHint*)prhPekiAmp;

		descsPekiAmp->instantiate = instantiatePekiAmp;
		descsPekiAmp->connect_port = connectPortToPekiAmp;
		descsPekiAmp->activate = NULL;
		descsPekiAmp->run = runStereoPekiAmp;
		descsPekiAmp->run_adding = NULL;
		descsPekiAmp->set_run_adding_gain = NULL;
		descsPekiAmp->deactivate = NULL;
		descsPekiAmp->cleanup = cleanupPekiAmp;

	}

}

static void deleteDescriptor(LADSPA_Descriptor* descPekiAmp) {

	unsigned long lIndex;

	if (descPekiAmp) {
		free((char*)descPekiAmp->Label);
		free((char*)descPekiAmp->Name);
		free((char*)descPekiAmp->Maker);
		free((char*)descPekiAmp->Copyright);
		free((LADSPA_PortDescriptor*)descPekiAmp->PortDescriptors);
		for (lIndex = 0; lIndex < descPekiAmp->PortCount; lIndex++)
			free((char*)(descPekiAmp->PortNames[lIndex]));
		free((char**)descPekiAmp->PortNames);
		free((LADSPA_PortRangeHint*)descPekiAmp->PortRangeHints);
		free(descPekiAmp);
	}

}

ON_UNLOAD_ROUTINE {
	deleteDescriptor(descmPekiAmp);
	deleteDescriptor(descsPekiAmp);
}

const LADSPA_Descriptor* ladspa_descriptor(unsigned long Index) {

	switch (Index) {
		case 0: return descmPekiAmp;
		case 1: return descsPekiAmp;
		default: return NULL;
	}

}
