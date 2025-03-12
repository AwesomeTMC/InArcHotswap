#pragma once

#include "InArcHotswap.h"
#include "SuperFlag.h"

namespace iah
{
    void *gInArcHotswapTbl = pt::loadArcAndFile("/ObjectData/InArcHotswap.arc", "/InArcHotswap.bcsv", 0);

    bool isCorrectFlagName(JMapInfo effectSwapTable, int index)
    {
        const char *pFlagName = "";
        MR::getCsvDataStr(&pFlagName, &effectSwapTable, "FlagName", index);
        return sf::isFlagActive(pFlagName);
    }

    bool isCorrectResourceName(JMapInfo effectSwapTable, int index, const char *pOriginalName)
    {
        const char *pUniqueName;
        MR::getCsvDataStr(&pUniqueName, &effectSwapTable, "ResourceName", index);
        return MR::isEqualString(pUniqueName, pOriginalName);
    }

    bool isCorrectIdentifier(JMapInfo tbl, int index, ResTable *pResTable)
    {
        const char *pIdentifierName;
        MR::getCsvDataStr(&pIdentifierName, &tbl, "Param01", index);
        if (MR::isNullOrEmptyString(pIdentifierName))
        {
            return true;
        }
        for (int i = 0; i < pResTable->mResCount; i++)
        {
            const char *resName = pResTable->getResName(i);
            if (resName[0] == '%')
            {
                resName++;

                if (MR::isEqualString(pIdentifierName, resName))
                {
                    return true;
                }
            }
        }
        return false;
    }

    const char *getSwappedResourceName(ResTable *pResTable, const char *pOriginalName)
    {
        if (MR::isNullOrEmptyString(pOriginalName))
        {
            return pOriginalName;
        }
        // Load arc and set variables.
        if (MR::isFileExist("/ObjectData/InArcHotswap.arc", false))
        {
            JMapInfo JMapTable = JMapInfo();
            JMapTable.attach(gInArcHotswapTbl);
            for (int i = 0; i < JMapTable.end().mIndex; i++)
            {
                if (isCorrectResourceName(JMapTable, i, pOriginalName) && isCorrectIdentifier(JMapTable, i, pResTable) && isCorrectFlagName(JMapTable, i))
                {

                    // return swapped name
                    const char *swappedName = "";
                    MR::getCsvDataStr(&swappedName, &JMapTable, "Param00Str", i);
                    // Param01
                    OSReport("IAHS-INFO: Swapping %s with %s!\n", pOriginalName, swappedName);
                    if (pResTable->getResIndex(swappedName) == -1)
                    {
                        OSReport("IAHS-WARN: Cannot swap; there is no resource with that name.\n");
                        return pOriginalName;
                    }
                    return swappedName;
                }
            }
        }
        else
        {
            OSReport("/ObjectData/InArcHotswap.arc doesn't exist and is not applied.\n");
        }

        return pOriginalName;
    }

    kmCall(0x804CB3A4, getSwappedResourceName);

    void *checkResName(ResTable *pTable, u32 index)
    {
        index = pTable->getResIndex(getSwappedResourceName(pTable, pTable->mFileInfo[index].mResName));
        return pTable->mFileInfo[index].mRes;
    }
    kmBranch(0x804CB290, checkResName);
}