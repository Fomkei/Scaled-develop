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
/** @file Test.h
 * @authors:
 *   Dimitry Khokhlov <dimitry@ethdev.com>
 * @date 2016
 */

#pragma once
#include "TestFace.h"

namespace dev {

namespace eth {
class Client;
}

namespace rpc {

class Test : public TestFace {
public:
    Test( eth::Client& _eth );
    virtual RPCModules implementedModules() const override {
        return RPCModules{ RPCModule{ "test", "1.0" } };
    }
    virtual std::string test_getLogHash( std::string const& _param1 ) override;
    virtual std::string test_importRawBlock( std::string const& _blockRLP ) override;
    virtual bool test_mineBlocks( int _number ) override;
    virtual bool test_modifyTimestamp( int _timestamp ) override;

private:
    eth::Client& m_eth;
};

}  // namespace rpc
}  // namespace dev
