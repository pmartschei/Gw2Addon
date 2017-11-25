#ifndef PLUGIN_BASE_STATE_H
#define PLUGIN_BASE_STATE_H
enum PluginBaseState {
	CREATED,
	CONTEXT_FINISHED,
	MOUSE_FOCUS_FINISHED,
	CODED_TEXT_FINISHED,
	DECODE_TEXT_FINISHED,
	SCANNER_FINISHED,
	FAILURE,
	INITIALIZED
};
#endif