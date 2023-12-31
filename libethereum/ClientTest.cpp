/*
    Modifications Copyright (C) 2018 SKALE Labs

    This file is part of cpp-ethereum.

    cpp-ethereum is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    cpp-ethereum is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file ClientTest.cpp
 * @author Dimitry Khokhlov <dimitry@ethdev.com>
 * @date 2016
 */

#include <libdevcore/CommonJS.h>
#include <libethashseal/Ethash.h>
#include <libethereum/ClientTest.h>
#include <libethereum/SkaleHost.h>
#include <boost/filesystem/path.hpp>
#include <future>

using namespace std;
using namespace dev;
using namespace dev::eth;
namespace fs = boost::filesystem;

ClientTest& dev::eth::asClientTest( Interface& _c ) {
    return dynamic_cast< ClientTest& >( _c );
}

ClientTest* dev::eth::asClientTest( Interface* _c ) {
    return &dynamic_cast< ClientTest& >( *_c );
}

ClientTest::ClientTest( ChainParams const& _params, int _networkID,
    std::shared_ptr< GasPricer > _gpForAdoption,
    std::shared_ptr< SnapshotManager > _snapshotManager,
    std::shared_ptr< InstanceMonitor > _instanceMonitor, fs::path const& _dbPath,
    WithExisting _forceAction, TransactionQueue::Limits const& _limits )
    : Client( _params, _networkID, _gpForAdoption, _snapshotManager, _instanceMonitor, _dbPath,
          _forceAction, _limits ) {}

ClientTest::~ClientTest() {
    m_signalled.notify_all();  // to wake up the thread from Client::doWork()
    terminate();
}

void ClientTest::modifyTimestamp( int64_t _timestamp ) {
    Block block( chainParams().accountStartNonce );
    DEV_READ_GUARDED( x_preSeal )
    block = m_preSeal;

    Transactions transactions;
    DEV_READ_GUARDED( x_postSeal )
    transactions = m_postSeal.pending();
    block.resetCurrent( _timestamp );

    DEV_WRITE_GUARDED( x_preSeal )
    m_preSeal = block;

    auto& lastHashes = bc().lastBlockHashes();
    assert( bc().currentHash() == block.info().parentHash() );
    for ( auto const& t : transactions )
        block.execute( lastHashes, t );

    DEV_WRITE_GUARDED( x_working )
    m_working = block;
    DEV_READ_GUARDED( x_postSeal )
    m_postSeal = block;

    onPostStateChanged();
}

bool ClientTest::mineBlocks( unsigned _count ) noexcept {
    std::cout << "mineBlocks begin " << _count << std::endl;
    if ( wouldSeal() )
        return false;
    try {
        std::promise< void > allBlocksImported;
        int blocksLeftToImport = _count;
        auto importHandler = setOnBlockImport(
            [this, &blocksLeftToImport, &allBlocksImported]( BlockHeader const& ) {
                if ( --blocksLeftToImport == 0 ) {
                    stopSealing();
                    allBlocksImported.set_value();
                }
            } );
        startSealing();
        future_status ret = allBlocksImported.get_future().wait_for(
            std::chrono::seconds( m_singleBlockMaxMiningTimeInSeconds * _count ) );
        std::cout << "mineBlocks end 0 is OK:" << ( int ) ret << std::endl;
        return ( ret == future_status::ready );
    } catch ( std::exception const& ) {
        LOG( m_logger ) << boost::current_exception_diagnostic_information();
        return false;
    }
}

h256 ClientTest::importRawBlock( const string& _blockRLP ) {
    bytes blockBytes = jsToBytes( _blockRLP, OnFailed::Throw );
    h256 blockHash = BlockHeader::headerHashFromBlock( blockBytes );
    ImportResult result = queueBlock( blockBytes, true );
    if ( result != ImportResult::Success )
        BOOST_THROW_EXCEPTION( ImportBlockFailed() << errinfo_importResult( result ) );

    // TOOD notify some sync component

    bool moreToImport = true;
    while ( moreToImport ) {
        tie( ignore, moreToImport, ignore ) = syncQueue( 100000 );
        this_thread::sleep_for( chrono::milliseconds( 100 ) );
    }
    return blockHash;
}
