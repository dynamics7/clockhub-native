// (C) ultrashot 2011-2013
// All rights reserved
#include "stdafx.h"
#include "CConditionalRender.hpp"
#include <stdarg.h>
#include <iostream>

CConditionalRender::CConditionalRender()
{
    _onChanged = NULL;
    memset(_conditions, 0, sizeof(_conditions));
    InitializeCriticalSection(&csSetCondition);
}

CConditionalRender::CConditionalRender(CONDITIONALRENDER_ONCHANGED onChanged)
{
    _onChanged = onChanged;
    memset(_conditions, 0, sizeof(_conditions));
    InitializeCriticalSection(&csSetCondition);
}

CConditionalRender::~CConditionalRender()
{
    DeleteCriticalSection(&csSetCondition);
}

void __cdecl CConditionalRender::SetCondition(int count, RenderConditionPair condition, ...)
{
    EnterCriticalSection(&csSetCondition);
    RenderConditionPair *pair = &condition;
    bool somethingChanged = false;
    for (int i = 0; i < count; ++i)
    {
        int index = GetIndex(pair->condition);
        if (index >= 0 && index < RenderCondition::Length)
        {
            if (_conditions[index] != pair->value)
            {
                _conditions[index] = pair->value;
                somethingChanged = true;
            }
        }
        pair += 1;
    }
    if (somethingChanged && _onChanged)
        _onChanged(this);
    LeaveCriticalSection(&csSetCondition);
}

bool CConditionalRender::GetConditionValue(RenderCondition::RenderCondition condition)
{
    int index = GetIndex(condition);
    if (index >= 0 && index < RenderCondition::Length)
    {
        return _conditions[index];
    }
    return false;
}

void CConditionalRender::SetCallback(CONDITIONALRENDER_ONCHANGED onChanged)
{
    _onChanged = onChanged;
}

int CConditionalRender::GetIndex(RenderCondition::RenderCondition condition)
{
    int result = 0;
    while (condition != 0 && condition != 1)
    {
        condition = (RenderCondition::RenderCondition)((unsigned int)condition >> 1);
        result++;
    }
    return result;
}

bool CConditionalRender::AllConditionsTrue()
{
    for (int i = 0; i < RenderCondition::Length; ++i)
    {
        if (_conditions[i] == false)
            return false;
    }
    return true;
}

CConditionalRender MainConditionalRender;
