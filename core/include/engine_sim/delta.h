#ifndef DELTA_STUB_H
#define DELTA_STUB_H

/* 
 * Stub header to allow engine-sim core code to compile without delta-studio.
 * 
 * This replaces the original delta.h which pulled in:
 *   <delta-studio/include/yds_core.h>
 *   <delta-studio/engines/basic/include/delta_basic_engine.h>
 * 
 * The synthesizer.cpp file does not actually use any delta/YDS types,
 * so this empty stub is sufficient for Android compilation.
 */

#endif /* DELTA_STUB_H */