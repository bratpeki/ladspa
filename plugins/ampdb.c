#include <math.h>
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

#define pow10f( x ) expf(2.302585092994046f*(x))

typedef struct {

	LADSPA_Data* pControl;

	LADSPA_Data* pInput1;
	LADSPA_Data* pOutput1;
	LADSPA_Data* pInput2;  /* Stereo */
	LADSPA_Data* pOutput2; /* Stereo */

} PekiAmpDB;

static LADSPA_Handle instantiatePekiAmpDB(
	const LADSPA_Descriptor* Descriptor,
	unsigned long SampleRate
) {
	return malloc(sizeof(PekiAmpDB));
}

static void connectPortToPekiAmpDB(
	LADSPA_Handle Instance,
	unsigned long Port,
	LADSPA_Data* DataLocation
) {

	PekiAmpDB* sPekiAmpDB;

	sPekiAmpDB = (PekiAmpDB*)Instance;

	switch (Port) {

		case AMP_CONTROL: sPekiAmpDB->pControl = DataLocation; break;
		case AMP_INPUT1:  sPekiAmpDB->pInput1  = DataLocation; break;
		case AMP_OUTPUT1: sPekiAmpDB->pOutput1 = DataLocation; break;
		case AMP_INPUT2:  sPekiAmpDB->pInput2  = DataLocation; break; /* Stereo */
		case AMP_OUTPUT2: sPekiAmpDB->pOutput2 = DataLocation; break; /* Stereo */

	}

}

static void runMonoPekiAmpDB(
	LADSPA_Handle Instance,
	 unsigned long SampleCount
) {

	LADSPA_Data* dInput;
	LADSPA_Data* dOutput;
	LADSPA_Data dGain;
	PekiAmpDB* sPekiAmpDB;
	unsigned long sampleIndex;

	sPekiAmpDB = (PekiAmpDB*)Instance;

	dInput = sPekiAmpDB->pInput1;
	dOutput = sPekiAmpDB->pOutput1;
	dGain = *(sPekiAmpDB->pControl);

	for (sampleIndex = 0; sampleIndex < SampleCount; ++sampleIndex)
		dOutput[sampleIndex] = dInput[sampleIndex] * pow10f(dGain / 20.0);

}

static void runStereoPekiAmpDB(
	LADSPA_Handle Instance,
	unsigned long SampleCount
) {

	LADSPA_Data* dInput;
	LADSPA_Data* dOutput;
	LADSPA_Data dGain;
	PekiAmpDB* sPekiAmpDB;
	unsigned long sampleIndex;

	sPekiAmpDB = (PekiAmpDB*)Instance;

	dGain = *(sPekiAmpDB->pControl);

	dInput = sPekiAmpDB->pInput1;
	dOutput = sPekiAmpDB->pOutput1;
	for (sampleIndex = 0; sampleIndex < SampleCount; ++sampleIndex)
		dOutput[sampleIndex] = dInput[sampleIndex] * pow10f(dGain / 20.0);

	dInput = sPekiAmpDB->pInput2;
	dOutput = sPekiAmpDB->pOutput2;
	for (sampleIndex = 0; sampleIndex < SampleCount; ++sampleIndex)
		dOutput[sampleIndex] = dInput[sampleIndex] * pow10f(dGain / 20.0);

}

static void cleanupPekiAmpDB( LADSPA_Handle Instance ) { free(Instance); }

LADSPA_Descriptor* descmPekiAmpDB = NULL;
LADSPA_Descriptor* descsPekiAmpDB = NULL;

ON_LOAD_ROUTINE {

	char ** pnPekiAmpDB;
	LADSPA_PortDescriptor* pdPekiAmpDB;
	LADSPA_PortRangeHint* prhPekiAmpDB;

	descmPekiAmpDB = (LADSPA_Descriptor*)malloc(sizeof(LADSPA_Descriptor));
	descsPekiAmpDB = (LADSPA_Descriptor*)malloc(sizeof(LADSPA_Descriptor));

	if (descmPekiAmpDB) {

		descmPekiAmpDB->UniqueID = 2001;

		descmPekiAmpDB->Label = strdup("peki_ampdb_mono");
		descmPekiAmpDB->Name = strdup("Peki's dB Amplifier (Mono)");
		descmPekiAmpDB->Maker = strdup("Petar Katić (bratpeki)");
		descmPekiAmpDB->Copyright = strdup("GPLv3");

		descmPekiAmpDB->PortCount = 3;
		descmPekiAmpDB->Properties = LADSPA_PROPERTY_HARD_RT_CAPABLE;

		pdPekiAmpDB = (LADSPA_PortDescriptor*)calloc(3, sizeof(LADSPA_PortDescriptor));
		pdPekiAmpDB[AMP_CONTROL] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
		pdPekiAmpDB[AMP_INPUT1] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
		pdPekiAmpDB[AMP_OUTPUT1] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
		descmPekiAmpDB->PortDescriptors = (const LADSPA_PortDescriptor*)pdPekiAmpDB;

		pnPekiAmpDB = (char **)calloc(3, sizeof(char*));
		pnPekiAmpDB[AMP_CONTROL] = strdup("Gain");
		pnPekiAmpDB[AMP_INPUT1] = strdup("Input");
		pnPekiAmpDB[AMP_OUTPUT1] = strdup("Output");
		descmPekiAmpDB->PortNames = (const char**)pnPekiAmpDB;

		prhPekiAmpDB = ((LADSPA_PortRangeHint*)calloc(3, sizeof(LADSPA_PortRangeHint)));
		prhPekiAmpDB[AMP_CONTROL].HintDescriptor = (
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_0
		);
		prhPekiAmpDB[AMP_CONTROL].LowerBound = -20;
		prhPekiAmpDB[AMP_CONTROL].UpperBound = 20;
		prhPekiAmpDB[AMP_INPUT1].HintDescriptor = 0;
		prhPekiAmpDB[AMP_OUTPUT1].HintDescriptor = 0;
		descmPekiAmpDB->PortRangeHints = (const LADSPA_PortRangeHint*)prhPekiAmpDB;

		descmPekiAmpDB->instantiate = instantiatePekiAmpDB;
		descmPekiAmpDB->connect_port = connectPortToPekiAmpDB;
		descmPekiAmpDB->activate = NULL;
		descmPekiAmpDB->run = runMonoPekiAmpDB;
		descmPekiAmpDB->run_adding = NULL;
		descmPekiAmpDB->set_run_adding_gain = NULL;
		descmPekiAmpDB->deactivate = NULL;
		descmPekiAmpDB->cleanup = cleanupPekiAmpDB;
	}

	if (descsPekiAmpDB) {

		descsPekiAmpDB->UniqueID = 2002;

		descsPekiAmpDB->Label = strdup("peki_ampdb_stereo");
		descsPekiAmpDB->Name = strdup("Peki's dB Amplifier (Stereo)");
		descsPekiAmpDB->Maker = strdup("Petar Katić (bratpeki)");
		descsPekiAmpDB->Copyright = strdup("GPLv3");

		descsPekiAmpDB->PortCount = 5;
		descsPekiAmpDB->Properties = LADSPA_PROPERTY_HARD_RT_CAPABLE;

		pdPekiAmpDB = (LADSPA_PortDescriptor*)calloc(5, sizeof(LADSPA_PortDescriptor));
		pdPekiAmpDB[AMP_CONTROL] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
		pdPekiAmpDB[AMP_INPUT1] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
		pdPekiAmpDB[AMP_OUTPUT1] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
		pdPekiAmpDB[AMP_INPUT2] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
		pdPekiAmpDB[AMP_OUTPUT2] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
		descsPekiAmpDB->PortDescriptors = (const LADSPA_PortDescriptor*)pdPekiAmpDB;

		pnPekiAmpDB = (char **)calloc(5, sizeof(char*));
		pnPekiAmpDB[AMP_CONTROL] = strdup("Gain");
		pnPekiAmpDB[AMP_INPUT1] = strdup("Input (Left)");
		pnPekiAmpDB[AMP_OUTPUT1] = strdup("Output (Left)");
		pnPekiAmpDB[AMP_INPUT2] = strdup("Input (Right)");
		pnPekiAmpDB[AMP_OUTPUT2] = strdup("Output (Right)");
		descsPekiAmpDB->PortNames = (const char**)pnPekiAmpDB;

		prhPekiAmpDB = ((LADSPA_PortRangeHint*)calloc(5, sizeof(LADSPA_PortRangeHint)));
		prhPekiAmpDB[AMP_CONTROL].HintDescriptor = (
			LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_0
		);
		prhPekiAmpDB[AMP_CONTROL].LowerBound = -20;
		prhPekiAmpDB[AMP_CONTROL].UpperBound = 20;
		prhPekiAmpDB[AMP_INPUT1].HintDescriptor = 0;
		prhPekiAmpDB[AMP_OUTPUT1].HintDescriptor = 0;
		prhPekiAmpDB[AMP_INPUT2].HintDescriptor = 0;
		prhPekiAmpDB[AMP_OUTPUT2].HintDescriptor = 0;
		descsPekiAmpDB->PortRangeHints = (const LADSPA_PortRangeHint*)prhPekiAmpDB;

		descsPekiAmpDB->instantiate = instantiatePekiAmpDB;
		descsPekiAmpDB->connect_port = connectPortToPekiAmpDB;
		descsPekiAmpDB->activate = NULL;
		descsPekiAmpDB->run = runStereoPekiAmpDB;
		descsPekiAmpDB->run_adding = NULL;
		descsPekiAmpDB->set_run_adding_gain = NULL;
		descsPekiAmpDB->deactivate = NULL;
		descsPekiAmpDB->cleanup = cleanupPekiAmpDB;

	}

}

static void deleteDescriptor(LADSPA_Descriptor* descPekiAmpDB) {

	unsigned long lIndex;

	if (descPekiAmpDB) {
		free((char*)descPekiAmpDB->Label);
		free((char*)descPekiAmpDB->Name);
		free((char*)descPekiAmpDB->Maker);
		free((char*)descPekiAmpDB->Copyright);
		free((LADSPA_PortDescriptor*)descPekiAmpDB->PortDescriptors);
		for (lIndex = 0; lIndex < descPekiAmpDB->PortCount; lIndex++)
			free((char*)(descPekiAmpDB->PortNames[lIndex]));
		free((char**)descPekiAmpDB->PortNames);
		free((LADSPA_PortRangeHint*)descPekiAmpDB->PortRangeHints);
		free(descPekiAmpDB);
	}

}

ON_UNLOAD_ROUTINE {
	deleteDescriptor(descmPekiAmpDB);
	deleteDescriptor(descsPekiAmpDB);
}

const LADSPA_Descriptor* ladspa_descriptor(unsigned long Index) {

	switch (Index) {
		case 0: return descmPekiAmpDB;
		case 1: return descsPekiAmpDB;
		default: return NULL;
	}

}
