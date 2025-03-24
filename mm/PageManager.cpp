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

//��ɢ����ʹ��λ��ͼ
#include "PageManager.h"

unsigned int PageManager::PHY_MEM_SIZE;
unsigned int UserPageManager::USER_PAGE_POOL_SIZE;

PageManager::PageManager(Allocator* allocator)
{
	this->m_pAllocator = allocator;
}

int PageManager::Initialize()
{
    // ��ʼ��λͼΪ0����ʾ����ҳ�򶼿��У�
    for(unsigned int i = 0; i < MAX_PAGES / BITS_PER_WORD; i++) {
        m_bitmap[i] = 0;
    }
	return 0;
}
////��������

//����λ
void PageManager::SetBit(unsigned int bitIndex)
{
    if (bitIndex >= MAX_PAGES) return; // �߽���
    unsigned int wordIndex = bitIndex / BITS_PER_WORD;
    unsigned int bit = bitIndex % BITS_PER_WORD;
    m_bitmap[wordIndex] |= (1U << bit);
}
//ɾ��λ
void PageManager::ClearBit(unsigned int bitIndex)
{
    if (bitIndex >= MAX_PAGES) return; // �߽���
    unsigned int wordIndex = bitIndex / BITS_PER_WORD;
    unsigned int bit = bitIndex % BITS_PER_WORD;
    m_bitmap[wordIndex] &= ~(1U << bit);
}
//�鿴λ�Ƿ��ѷ���
bool PageManager::TestBit(unsigned int bitIndex)
{
    if (bitIndex >= MAX_PAGES) return false; // �߽���
    unsigned int wordIndex = bitIndex / BITS_PER_WORD;
    unsigned int bit = bitIndex % BITS_PER_WORD;
    return (m_bitmap[wordIndex] & (1U << bit)) != 0;
}
//�����Ƿ�����㹻������ҳ
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
    return -1;  // û���ҵ��㹻������ҳ
}

////��������

//�޸�
unsigned long PageManager::AllocMemory(unsigned long size)
{
	//�߽���
	if(size == 0) return 0;

    // ������Ҫ��ҳ��
    unsigned int numPages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    // ���������Ŀ���ҳ
    int startPage = FindContiguousPages(numPages);
    if(startPage < 0) return 0;  // ����ʧ��

    // �����ЩҳΪ�ѷ���
    for(unsigned int i = startPage; i < startPage + numPages; i++) {
        SetBit(i);
    }

    // ������ʼ�����ַ
    return (unsigned long)(startPage * PAGE_SIZE);
}

//�޸�
unsigned long PageManager::FreeMemory(unsigned long size, unsigned long startAddress)
{
    if (size == 0 || startAddress % PAGE_SIZE != 0) return 0; // �߽���

	// ������ʼҳ�ź�ҳ��
    unsigned int startPage = startAddress / PAGE_SIZE;
    unsigned int numPages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    // �����Ӧ��λͼλ
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

//�޸�
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

//�޸�
int UserPageManager::Initialize()
{
	PageManager::Initialize();
    m_startPage = USER_PAGE_POOL_START_ADDR / PAGE_SIZE;
    m_totalPages = USER_PAGE_POOL_SIZE / PAGE_SIZE;
	return 0;
}
