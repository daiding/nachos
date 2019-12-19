#include "system.h"
#include "swapLRU.h"

SwapLRU::SwapLRU(int pageNumber)
{
    pageLastUsedTime = new int[pageNumber];
    pageNum = pageNumber;
}

SwapLRU::~SwapLRU()
{
    delete pageLastUsedTime;
}

void SwapLRU::UpdatePageLastUsedTime(int pageNO)
{
    pageLastUsedTime[pageNO] = stats->totalTicks;
    return;
}

int SwapLRU::FindOnePageToSwap()
{
    int minTimeTick = pageLastUsedTime[0];
    int longestTimeNotUsedPageNO = 0;
    for (int i = 1; i < pageNum; i++)
    {
        if (minTimeTick > pageLastUsedTime[i])
        {
            minTimeTick = pageLastUsedTime[i];
            longestTimeNotUsedPageNO = i;
        }
    }
    return longestTimeNotUsedPageNO;
}
