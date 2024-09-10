#include <stdlib.h>
#include <string.h>

#include <ladspa.h>

#define AMP_CONTROL 0
#define AMP_INPUT1 1
#define AMP_OUTPUT1 2
#define AMP_INPUT2 3
#define AMP_OUTPUT2 4

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

typedef struct {

	LADSPA_Data* m_pfControlValue;
	LADSPA_Data* m_pfInputBuffer1;
	LADSPA_Data* m_pfOutputBuffer1;
	LADSPA_Data* m_pfInputBuffer2;  /* Stereo */
	LADSPA_Data* m_pfOutputBuffer2; /* Stereo */

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

	Amplifier* psAmplifier;

	psAmplifier = (Amplifier*)Instance;

	switch (Port) {

		case AMP_CONTROL: psAmplifier->m_pfControlValue  = DataLocation; break;
		case AMP_INPUT1:  psAmplifier->m_pfInputBuffer1  = DataLocation; break;
		case AMP_OUTPUT1: psAmplifier->m_pfOutputBuffer1 = DataLocation; break;
		case AMP_INPUT2:  psAmplifier->m_pfInputBuffer2  = DataLocation; break; /* Stereo */
		case AMP_OUTPUT2: psAmplifier->m_pfOutputBuffer2 = DataLocation; break; /* Stereo */

	}

}

static void runMonoAmplifier(
	LADSPA_Handle Instance,
	 unsigned long SampleCount
) {

	LADSPA_Data* pfInput;
	LADSPA_Data* pfOutput;
	LADSPA_Data fGain;
	Amplifier* psAmplifier;
	unsigned long lSampleIndex;

	psAmplifier = (Amplifier*)Instance;

	pfInput = psAmplifier->m_pfInputBuffer1;
	pfOutput = psAmplifier->m_pfOutputBuffer1;
	fGain = *(psAmplifier->m_pfControlValue);

	for (lSampleIndex = 0; lSampleIndex < SampleCount; lSampleIndex++)
		*(pfOutput++) = *(pfInput++)* fGain;

}

static void runStereoAmplifier(
	LADSPA_Handle Instance,
	unsigned long SampleCount
) {

	LADSPA_Data* pfInput;
	LADSPA_Data* pfOutput;
	LADSPA_Data fGain;
	Amplifier* psAmplifier;
	unsigned long lSampleIndex;

	psAmplifier = (Amplifier*)Instance;

	fGain = *(psAmplifier->m_pfControlValue);

	pfInput = psAmplifier->m_pfInputBuffer1;
	pfOutput = psAmplifier->m_pfOutputBuffer1;
	for (lSampleIndex = 0; lSampleIndex < SampleCount; lSampleIndex++)
		*(pfOutput++) = *(pfInput++)* fGain;

	pfInput = psAmplifier->m_pfInputBuffer2;
	pfOutput = psAmplifier->m_pfOutputBuffer2;
	for (lSampleIndex = 0; lSampleIndex < SampleCount; lSampleIndex++)
		*(pfOutput++) = *(pfInput++)* fGain;

}

static void cleanupAmplifier( LADSPA_Handle Instance ) { free(Instance); }

LADSPA_Descriptor* g_psMonoDescriptor = NULL;
LADSPA_Descriptor* g_psStereoDescriptor = NULL;

ON_LOAD_ROUTINE {

	char ** pcPortNames;
	LADSPA_PortDescriptor* piPortDescriptors;
	LADSPA_PortRangeHint* psPortRangeHints;

	g_psMonoDescriptor = (LADSPA_Descriptor*)malloc(sizeof(LADSPA_Descriptor));
	g_psStereoDescriptor = (LADSPA_Descriptor*)malloc(sizeof(LADSPA_Descriptor));

	if (g_psMonoDescriptor) {

		g_psMonoDescriptor->UniqueID = 1;

		g_psMonoDescriptor->Label = strdup("peki_amp_mono");
		g_psMonoDescriptor->Name = strdup("Peki's Mono Amplifier");
		g_psMonoDescriptor->Maker = strdup("Petar Katić (bratpeki)");
		g_psMonoDescriptor->Copyright = strdup("None");

		g_psMonoDescriptor->PortCount = 3;
		g_psMonoDescriptor->Properties = LADSPA_PROPERTY_HARD_RT_CAPABLE;

		piPortDescriptors = (LADSPA_PortDescriptor*)calloc(3, sizeof(LADSPA_PortDescriptor));
		piPortDescriptors[AMP_CONTROL] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
		piPortDescriptors[AMP_INPUT1] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
		piPortDescriptors[AMP_OUTPUT1] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
		g_psMonoDescriptor->PortDescriptors = (const LADSPA_PortDescriptor*)piPortDescriptors;

		pcPortNames = (char **)calloc(3, sizeof(char*));
		pcPortNames[AMP_CONTROL] = strdup("Gain");
		pcPortNames[AMP_INPUT1] = strdup("Input");
		pcPortNames[AMP_OUTPUT1] = strdup("Output");
		g_psMonoDescriptor->PortNames = (const char **)pcPortNames;

		psPortRangeHints = ((LADSPA_PortRangeHint*)calloc(3, sizeof(LADSPA_PortRangeHint)));
		psPortRangeHints[AMP_CONTROL].HintDescriptor = (
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_LOGARITHMIC | LADSPA_HINT_DEFAULT_1
		);
		psPortRangeHints[AMP_CONTROL].LowerBound = 0;
		psPortRangeHints[AMP_INPUT1].HintDescriptor = 0;
		psPortRangeHints[AMP_OUTPUT1].HintDescriptor = 0;
		g_psMonoDescriptor->PortRangeHints = (const LADSPA_PortRangeHint*)psPortRangeHints;

		g_psMonoDescriptor->instantiate = instantiateAmplifier;
		g_psMonoDescriptor->connect_port = connectPortToAmplifier;
		g_psMonoDescriptor->activate = NULL;
		g_psMonoDescriptor->run = runMonoAmplifier;
		g_psMonoDescriptor->run_adding = NULL;
		g_psMonoDescriptor->set_run_adding_gain = NULL;
		g_psMonoDescriptor->deactivate = NULL;
		g_psMonoDescriptor->cleanup = cleanupAmplifier;
	}

	if (g_psStereoDescriptor) {

		g_psStereoDescriptor->UniqueID = 2;

		g_psStereoDescriptor->Label = strdup("peki_amp_stereo");
		g_psStereoDescriptor->Name = strdup("Peki's Stereo Amplifier");
		g_psStereoDescriptor->Maker = strdup("Petar Katić (bratpeki)");
		g_psStereoDescriptor->Copyright = strdup("None");

		g_psStereoDescriptor->PortCount = 5;
		g_psStereoDescriptor->Properties = LADSPA_PROPERTY_HARD_RT_CAPABLE;

		piPortDescriptors = (LADSPA_PortDescriptor*)calloc(5, sizeof(LADSPA_PortDescriptor));
		piPortDescriptors[AMP_CONTROL] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
		piPortDescriptors[AMP_INPUT1] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
		piPortDescriptors[AMP_OUTPUT1] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
		piPortDescriptors[AMP_INPUT2] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
		piPortDescriptors[AMP_OUTPUT2] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
		g_psStereoDescriptor->PortDescriptors = (const LADSPA_PortDescriptor*)piPortDescriptors;

		pcPortNames = (char **)calloc(5, sizeof(char*));
		pcPortNames[AMP_CONTROL] = strdup("Gain");
		pcPortNames[AMP_INPUT1] = strdup("Input (Left)");
		pcPortNames[AMP_OUTPUT1] = strdup("Output (Left)");
		pcPortNames[AMP_INPUT2] = strdup("Input (Right)");
		pcPortNames[AMP_OUTPUT2] = strdup("Output (Right)");
		g_psStereoDescriptor->PortNames = (const char **)pcPortNames;

		psPortRangeHints = ((LADSPA_PortRangeHint*)calloc(5, sizeof(LADSPA_PortRangeHint)));
		psPortRangeHints[AMP_CONTROL].HintDescriptor = (
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_LOGARITHMIC | LADSPA_HINT_DEFAULT_1
		);
		psPortRangeHints[AMP_CONTROL].LowerBound = 0;
		psPortRangeHints[AMP_INPUT1].HintDescriptor = 0;
		psPortRangeHints[AMP_OUTPUT1].HintDescriptor = 0;
		psPortRangeHints[AMP_INPUT2].HintDescriptor = 0;
		psPortRangeHints[AMP_OUTPUT2].HintDescriptor = 0;
		g_psStereoDescriptor->PortRangeHints = (const LADSPA_PortRangeHint*)psPortRangeHints;

		g_psStereoDescriptor->instantiate = instantiateAmplifier;
		g_psStereoDescriptor->connect_port = connectPortToAmplifier;
		g_psStereoDescriptor->activate = NULL;
		g_psStereoDescriptor->run = runStereoAmplifier;
		g_psStereoDescriptor->run_adding = NULL;
		g_psStereoDescriptor->set_run_adding_gain = NULL;
		g_psStereoDescriptor->deactivate = NULL;
		g_psStereoDescriptor->cleanup = cleanupAmplifier;

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
	deleteDescriptor(g_psMonoDescriptor);
	deleteDescriptor(g_psStereoDescriptor);
}

const LADSPA_Descriptor* ladspa_descriptor(unsigned long Index) {
	switch (Index) {
		case 0: return g_psMonoDescriptor;
		case 1: return g_psStereoDescriptor;
		default: return NULL;
	}
}
