// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ComponentVisualizer.h"

class FSpringComponentVisualizer : public FComponentVisualizer
{
public:
	// Begin FComponentVisualizer interface
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	// End FComponentVisualizer interface
};
