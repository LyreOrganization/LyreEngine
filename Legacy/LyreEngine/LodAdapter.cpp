#include "stdafx.h"
#include "LodAdapter.h"

#include "LyreEngine.h"

using namespace std;
using namespace std::chrono_literals;

LodAdapter::LodAdapter(SpherifiedCube* pSphere)
	: m_pSphere(pSphere), m_bStop(false), m_bNewData(false) {}

LodAdapter::~LodAdapter() {
	{
		unique_lock<mutex> locker(m_membersLock);
		m_bStop = true;
	}
	m_cvOnRead.notify_all();
	if (m_tAdapter.joinable()) m_tAdapter.join();
}

void LodAdapter::start() {
	m_tAdapter = thread([this]() {
		auto lastUpdateTime = chrono::high_resolution_clock::now();
		bool bUpdate = false;
		while (true) {
			unique_lock<mutex> locker(m_membersLock);
			m_cvOnRead.wait_for(locker, 5000ms, [this, &lastUpdateTime, &bUpdate]() {
				if (m_bStop) return true;

				auto nowTime = chrono::high_resolution_clock::now();
				if (chrono::duration_cast<chrono::milliseconds>(
					nowTime - lastUpdateTime) > 5000ms) {
					lastUpdateTime = nowTime;
					return (bUpdate = true);
				}

				return m_bNewData;
			});
			if (m_bStop) return;

			if (m_bNewData) {
				auto lodData = move(m_lodData);
				m_bNewData = false;

				locker.unlock();

				OutputDebugStringA("Adapt\n");

				{
					for (auto it = lodData.lower_bound(0.f); it != lodData.end(); ++it) {
						if (it->first > 0.5f) break;
						auto* plane = m_pSphere->getPlane(it->second);
						if (plane) plane->tryDivide();
					}
				}
				{
					auto it = lodData.rbegin();
					for (int i = 0; i < 2000 && it != lodData.rend(); ++it, i++) {
						if (it->first < 4.f) break;
						
						// take parent
						if (it->second.level > 0) {
							it->second.level--;
							it->second.position.x >>= 1;
							it->second.position.y >>= 1;
						}
							
						auto* plane = m_pSphere->getPlane(it->second);
						if (plane) plane->tryUndivide();
					}
				}
			}
			else {
				locker.unlock();
			}

			if (bUpdate) {
				m_pSphere->loadTopology();
				bUpdate = false;
				OutputDebugStringA("Update\n");
			}
		}
	});
}

void LodAdapter::readCurrentLods(ID3D11UnorderedAccessView* pUav,
								 const SphereTopology& topology) {
	unique_lock<mutex> locker(m_membersLock, try_to_lock);
	if (locker) {
		int numPatches = static_cast<int>(topology.indices.size()) / 5;

		CComPtr<ID3D11Buffer> lodDiffBuff = nullptr;
		D3D11_BUFFER_DESC buffDesc;
		{
			ZeroStruct(buffDesc);
			// next multiple of 16
			buffDesc.ByteWidth = (((numPatches * sizeof(float)) >> 4) + 1) << 4;
			buffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			buffDesc.Usage = D3D11_USAGE_STAGING;
		}
		HRESULT hr = LyreEngine::getDevice()->CreateBuffer(
			&buffDesc, nullptr, &lodDiffBuff);
		if (FAILED(hr)) return;

		CComPtr<ID3D11Resource> uavBuff = nullptr;
		pUav->GetResource(&uavBuff);
		D3D11_BOX box;
		{
			ZeroStruct(box);
			box.right = numPatches * sizeof(float);
			box.bottom = 1;
			box.back = 1;
		}
		LyreEngine::getContext()->CopySubresourceRegion(
			lodDiffBuff, 0, 0, 0, 0, uavBuff, 0, &box);

		D3D11_MAPPED_SUBRESOURCE mappedLodDiffBuffer;
		hr = LyreEngine::getContext()->Map(
			lodDiffBuff, 0, D3D11_MAP_READ, 0, &mappedLodDiffBuffer);
		if (FAILED(hr)) return;

		m_lodData.clear();
		float* rawData = reinterpret_cast<float*>(mappedLodDiffBuffer.pData);
		for (int i = 0; i < numPatches; i++) {
			m_lodData.emplace(rawData[i], topology.planes[topology.indices[5*i]]);
		}

		LyreEngine::getContext()->Unmap(lodDiffBuff, 0);

		// notify
		m_bNewData = true;
		m_cvOnRead.notify_all();
	}
}
