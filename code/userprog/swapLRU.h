#ifndef SWAPLRU_H
#define SWAPLRU_H



class SwapLRU
{
    public:
    SwapLRU(int pageNum);
    ~SwapLRU();
    void UpdatePageLastUsedTime(int pageNO);
    int FindOnePageToSwap();
    private:
    int* pageLastUsedTime;
};

#endif
