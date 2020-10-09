#include "NdiManager.h"

NdiManager::NdiManager(const char* srcName, const char* dstName)
{
	if (!NDIlib_initialize()) assert(0);

	NDIlib_find_instance_t findInstance = NDIlib_find_create_v2();
	if (!findInstance)
		assert(0);

	uint32_t no_sources = 0;
	bool found = false;

	while (!found)
	{	
		const NDIlib_source_t* p_sources = NDIlib_find_get_current_sources(findInstance, &no_sources);

		// Display all the sources.
		for (uint32_t i = 0; i < no_sources; i++)
		{
			if (strcmp(srcName, p_sources[i].p_ndi_name))
			{
				found = true;

				m_recvInstance = NDIlib_recv_create_v3();
				if (!m_recvInstance) 
					assert(0);

				NDIlib_recv_connect(m_recvInstance, p_sources + i);

				printf("Source %s, found and connected!\n", p_sources[i].p_ndi_name);
			}
		}

		if (!found)
		{
			printf("Source %s not found. Waiting 1000ms before trying again!\n", srcName);

			NDIlib_find_wait_for_sources(findInstance, 1000);
		}
	}

	NDIlib_find_destroy(findInstance);

	NDIlib_send_create_t NDI_send_create_desc;
	NDI_send_create_desc.p_ndi_name = dstName;

	m_sendInstance = NDIlib_send_create(&NDI_send_create_desc);
	if (!m_sendInstance) 
		assert(0);
}

NdiManager::~NdiManager()
{
	NDIlib_recv_destroy(m_recvInstance);
	NDIlib_send_destroy(m_sendInstance);

	NDIlib_destroy();
}

NDIlib_recv_instance_t* NdiManager::getRecever()
{
	return &m_recvInstance;
}

NDIlib_send_instance_t* NdiManager::getSender()
{
	return &m_sendInstance;
}
