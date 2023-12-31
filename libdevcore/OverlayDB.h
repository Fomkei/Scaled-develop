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
/** @file OverlayDB.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <libdevcore/Common.h>
#include <libdevcore/Log.h>
#include <libdevcore/MemoryDB.h>
#include <libdevcore/db.h>
#include <memory>

namespace dev {

class OverlayDB : public MemoryDB {
public:
    explicit OverlayDB( std::unique_ptr< db::DatabaseFace > _db = nullptr )
        : m_db( _db.release(), []( db::DatabaseFace* db ) { delete db; } ) {}

    ~OverlayDB();

    // Copyable
    OverlayDB( OverlayDB const& ) = default;
    OverlayDB& operator=( OverlayDB const& ) = default;
    // Movable
    OverlayDB( OverlayDB&& ) = default;
    OverlayDB& operator=( OverlayDB&& ) = default;

    void commit();
    void rollback();

    void insert( h256 const& _h, bytesConstRef _v ) override;

    std::string lookup( h256 const& _h ) const;
    bool exists( h256 const& _h ) const;
    void kill( h256 const& _h );

    bytes lookupAux( h256 const& _h ) const;

    void setCommitOnEveryInsert( bool _value ) {
        commit();
        m_commitOnEveryInsert = _value;
    }

private:
    using MemoryDB::clear;
    std::shared_ptr< db::DatabaseFace > m_db;

    // a flag to commit to disk on every insert to save memory
    // this is currently only used for historic state conversion

    bool m_commitOnEveryInsert = false;
};

}  // namespace dev
