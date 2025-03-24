//#include "PageManager.h"
//#include "Allocator.h"
//
//unsigned int PageManager::PHY_MEM_SIZE;
//unsigned int UserPageManager::USER_PAGE_POOL_SIZE;
//
//PageManager::PageManager(Allocator* allocator)
//{
//	this->m_pAllocator = allocator;
//}
//
//int PageManager::Initialize()
//{
//	for ( unsigned int i = 0; i < MEMORY_MAP_ARRAY_SIZE; i++ )
//	{
//		this->map[i].m_AddressIdx = 0;
//		this->map[i].m_Size = 0;
//	}
//	return 0;
//}
//
//unsigned long PageManager::AllocMemory(unsigned long size)
//{
//	return this->m_pAllocator->Alloc(this->map,
//				(size + (PAGE_SIZE -1)) / PAGE_SIZE ) * PAGE_SIZE;
//}
//
//unsigned long PageManager::FreeMemory(unsigned long size, unsigned long startAddress)
//{
//	return this->m_pAllocator->Free(this->map,
//				(size + (PAGE_SIZE -1)) / PAGE_SIZE, startAddress / PAGE_SIZE);
//}
//
//PageManager::~PageManager()
//{
//}
//
//KernelPageManager::KernelPageManager(Allocator* allocator)
//	:PageManager(allocator)
//{
//}
//
//int KernelPageManager::Initialize()
//{
//	PageManager::Initialize();
//
//	this->map[0].m_AddressIdx =
//		KERNEL_PAGE_POOL_START_ADDR / PageManager::PAGE_SIZE;
//	this->map[0].m_Size =
//		KERNEL_PAGE_POOL_SIZE / PageManager::PAGE_SIZE;
//	return 0;
//}
//
//UserPageManager::UserPageManager(Allocator* allocator)
//	:PageManager(allocator)
//{
//}
//
//int UserPageManager::Initialize()
//{
//	PageManager::Initialize();
//
//	this->map[0].m_AddressIdx =
//		USER_PAGE_POOL_START_ADDR / PageManager::PAGE_SIZE;
//	this->map[0].m_Size =
//		USER_PAGE_POOL_SIZE / PageManager::PAGE_SIZE;
//	return 0;
//}

//离散化后使用位视图
#include "PageManager.h"

unsigned int PageManager::PHY_MEM_SIZE;
unsigned int UserPageManager::USER_PAGE_POOL_SIZE;

PageManager::PageManager(Allocator* allocator)
{
	this->m_pAllocator = allocator;
}

int PageManager::Initialize()
{
    // 初始化位图为0（表示所有页框都空闲）
    for(unsigned int i = 0; i < MAX_PAGES / BITS_PER_WORD; i++) {
        m_bitmap[i] = 0;
    }
	return 0;
}
////新增函数

//分配位
void PageManager::SetBit(unsigned int bitIndex)
{
    if (bitIndex >= MAX_PAGES) return; // 边界检查
    unsigned int wordIndex = bitIndex / BITS_PER_WORD;
    unsigned int bit = bitIndex % BITS_PER_WORD;
    m_bitmap[wordIndex] |= (1U << bit);
}
//删除位
void PageManager::ClearBit(unsigned int bitIndex)
{
    if (bitIndex >= MAX_PAGES) return; // 边界检查
    unsigned int wordIndex = bitIndex / BITS_PER_WORD;
    unsigned int bit = bitIndex % BITS_PER_WORD;
    m_bitmap[wordIndex] &= ~(1U << bit);
}
//查看位是否已分配
bool PageManager::TestBit(unsigned int bitIndex)
{
    if (bitIndex >= MAX_PAGES) return false; // 边界检查
    unsigned int wordIndex = bitIndex / BITS_PER_WORD;
    unsigned int bit = bitIndex % BITS_PER_WORD;
    return (m_bitmap[wordIndex] & (1U << bit)) != 0;
}
//查找是否存在足够的连续页
int PageManager::FindContiguousPages(unsigned int numPages)
{
    unsigned int count = 0;
    unsigned int startPage = 0;

    for(unsigned int i = m_startPage; i < m_startPage + m_totalPages; i++) {
        if(!TestBit(i)) {
            if(count == 0) startPage = i;
            count++;
            if(count == numPages) return startPage;
        } else {
            count = 0;
        }
    }
    return -1;  // 没有找到足够的连续页
}

////结束新增

//修改
unsigned long PageManager::AllocMemory(unsigned long size)
{
	//边界检查
	if(size == 0) return 0;

    // 计算需要的页数
    unsigned int numPages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    // 查找连续的空闲页
    int startPage = FindContiguousPages(numPages);
    if(startPage < 0) return 0;  // 分配失败

    // 标记这些页为已分配
    for(unsigned int i = startPage; i < startPage + numPages; i++) {
        SetBit(i);
    }

    // 返回起始物理地址
    return (unsigned long)(startPage * PAGE_SIZE);
}

//修改
unsigned long PageManager::FreeMemory(unsigned long size, unsigned long startAddress)
{
    if (size == 0 || startAddress % PAGE_SIZE != 0) return 0; // 边界检查

	// 计算起始页号和页数
    unsigned int startPage = startAddress / PAGE_SIZE;
    unsigned int numPages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    // 清除对应的位图位
    for(unsigned int i = startPage; i < startPage + numPages; i++) {
        ClearBit(i);
    }

    return 0;
}

PageManager::~PageManager()
{
}

KernelPageManager::KernelPageManager(Allocator* allocator)
	:PageManager(allocator)
{
}

//修改
int KernelPageManager::Initialize()
{
	PageManager::Initialize();
	
    m_startPage = KERNEL_PAGE_POOL_START_ADDR / PAGE_SIZE;
    m_totalPages = KERNEL_PAGE_POOL_SIZE / PAGE_SIZE;
	return 0;
}

UserPageManager::UserPageManager(Allocator* allocator)
	:PageManager(allocator)
{
}

//修改
int UserPageManager::Initialize()
{
	PageManager::Initialize();
    m_startPage = USER_PAGE_POOL_START_ADDR / PAGE_SIZE;
    m_totalPages = USER_PAGE_POOL_SIZE / PAGE_SIZE;
	return 0;
}
