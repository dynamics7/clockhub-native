// (C) ultrashot 2011-2013
// All rights reserved
#pragma once
#ifndef CCONDITIONALRENDER_HPP
#define CCONDITIONALRENDER_HPP

#include "windows.h"

#define RENDER_CONDITION_COUNT 3
namespace RenderCondition
{

    enum RenderCondition : unsigned int
    {
        Unknown = 0,
        IsOnTop = 1,
        IsUnlocked = 2,
        IsPinned = 4,
        Length = RENDER_CONDITION_COUNT
    };
}

typedef struct RenderConditionPair
{
    bool value;
    RenderCondition::RenderCondition condition;
    RenderConditionPair()
    {
        value = 0;
    }
    RenderConditionPair(unsigned int dwRenderCondition, bool Value)
    {
        condition = (RenderCondition::RenderCondition)dwRenderCondition;;
        value = Value;
    }
}RenderConditionPair;

class CConditionalRender;

typedef void (*CONDITIONALRENDER_ONCHANGED)(CConditionalRender *render);

class CConditionalRender
{
public:
    CConditionalRender();
    CConditionalRender(CONDITIONALRENDER_ONCHANGED onChanged);
    ~CConditionalRender();

    void SetCondition(int count, RenderConditionPair conditionPair, ...);
    bool GetConditionValue(RenderCondition::RenderCondition condition);
    void SetCallback(CONDITIONALRENDER_ONCHANGED onChanged);
    bool AllConditionsTrue();
private:
    bool _conditions[RENDER_CONDITION_COUNT];
    CONDITIONALRENDER_ONCHANGED _onChanged;
    int GetIndex(RenderCondition::RenderCondition condition);
    CRITICAL_SECTION csSetCondition;
};

extern CConditionalRender MainConditionalRender;

#endif
