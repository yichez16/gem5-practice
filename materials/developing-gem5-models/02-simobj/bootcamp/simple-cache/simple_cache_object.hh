/*
 * Copyright (c) 2021 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __BOOTCAMP_SIMPLE_CACHE_SIMPLE_CACHE_SIMPLE_CACHE_OBJECT_HH__
#define __BOOTCAMP_SIMPLE_CACHE_SIMPLE_CACHE_SIMPLE_CACHE_OBJECT_HH__

#include <unordered_map>

#include "base/statistics.hh"
#include "mem/port.hh"
#include "params/SimpleCache.hh"
#include "sim/clocked_object.hh"

namespace gem5
{

class SimpleCacheObject : public ClockedObject
{
  private:

    class CPUSidePort : public ResponsePort
    {
      private:
        // TODO: Expalin why this is needed.
        int id;
        SimpleCache *owner;
        bool needRetry;
        PacketPtr blockedPacket;

      public:
        CPUSidePort(const std::string& name, int id, SimpleCache *owner) :
            ResponsePort(name, owner), id(id), owner(owner), needRetry(false),
            blockedPacket(nullptr)
        { }

        void sendPacket(PacketPtr pkt);
        AddrRangeList getAddrRanges() const override;
        void trySendRetry();

      protected:
        Tick recvAtomic(PacketPtr pkt) override
        { panic("recvAtomic unimpl."); }

        void recvFunctional(PacketPtr pkt) override;
        bool recvTimingReq(PacketPtr pkt) override;
        void recvRespRetry() override;
    };

    class MemSidePort : public RequestPort
    {
      private:
        SimpleCache *owner;
        PacketPtr blockedPacket;

      public:
        MemSidePort(const std::string& name, SimpleCache *owner) :
            RequestPort(name, owner), owner(owner), blockedPacket(nullptr)
        { }

        void sendPacket(PacketPtr pkt);

      protected:
        bool recvTimingResp(PacketPtr pkt) override;
        void recvReqRetry() override;
        void recvRangeChange() override;
    };

    bool handleRequest(PacketPtr pkt, int port_id);
    bool handleResponse(PacketPtr pkt);

    // TODO: define void sendResponse(PacketPtr pkt);

    // TODO: define void handleFunctional(PacketPtr pkt);

    // TODO: define void accessTiming(PacketPtr pkt);

    // TODO: define bool accessFunctional(PacketPtr pkt);

    // TODO: void insert(PacketPtr pkt);

    AddrRangeList getAddrRanges() const;
    void sendRangeChange() const;

    const Cycles latency;
    const unsigned blockSize;
    const unsigned capacity;

    std::vector<CPUSidePort> cpuPorts;
    MemSidePort memPort;
    bool blocked;

    /// Packet that we are currently handling. Used for upgrading to larger
    /// cache line sizes
    PacketPtr originalPacket;
    int waitingPortId;
    Tick missTime;

    // TODO: std::unordered_map<Addr, uint8_t*> cacheStore;

  protected:
    struct SimpleCacheObjectStats : public statistics::Group
    {
        SimpleCacheObjectStats(statistics::Group *parent);
        statistics::Scalar hits;
        statistics::Scalar misses;
        statistics::Histogram missLatency;
        statistics::Formula hitRatio;
    } stats;

  public:
    PARAMS(SimpleCacheObject);
    SimpleCacheObject(const Params& params);

    Port &getPort(const std::string &if_name,
                  PortID idx=InvalidPortID) override;

};

} // namespace gem5

#endif // __BOOTCAMP_SIMPLE_CACHE_SIMPLE_CACHE_SIMPLE_CACHE_OBJECT_HH__
