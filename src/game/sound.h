#pragma once
#include <raylib.h>
#include "mask.h"

// lifecycle
void SoundInit();
void SoundShutdown();

// music
void SoundPlayMusic();
void SoundPauseMusic();
void SoundUpdate();

// events
void SoundOnDeath();
void SoundOnMoveStart(MaskType mask);
void SoundOnMoveStop(MaskType mask);
void SoundOnPlate();
void SoundOnMaskSwitch(MaskType mask);
void SoundStopMovement();
void SoundRestartMusic();
