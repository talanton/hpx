//  Copyright (c) 2007-2008 Hartmut Kaiser
//
//  Parts of this code were taken from the Boost.Asio library
//  Copyright (c) 2003-2007 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <string>

#include <boost/cstdint.hpp>
#include <boost/assert.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/detail/lightweight_test.hpp>

#include <hpx/hpx.hpp>

int main(int argc, char* argv[])
{
    // Check command line arguments.
    std::string host;
    unsigned short port;
    if (argc != 3)
    {
        std::cerr << "Using default settings: localhost:7911" << std::endl;
        std::cerr << "Possible arguments: <DGAS address> <DGAS port>" << std::endl;

        host = "localhost";
        port = 7911;
    }
    else
    {
        host = argv[1];
        port = boost::lexical_cast<unsigned short>(argv[2]);
    }

    try {
        using namespace hpx::naming;
        
        // this is our locality
        locality here("localhost", HPX_PORT);
        resolver_client resolver(host, port);
        
        // retrieve the id prefix of this site
        boost::uint64_t prefix1 = 0;
        BOOST_TEST(resolver.get_prefix(here, prefix1));
        BOOST_TEST(0 != prefix1);
        
        boost::uint64_t prefix2 = 0;
        BOOST_TEST(!resolver.get_prefix(here, prefix2));
        BOOST_TEST(prefix2 == prefix1);   // same site should get same prefix

        // different sites should get different prefix
        boost::uint64_t prefix3 = 0;
        BOOST_TEST(resolver.get_prefix(locality("1.1.1.1", 1), prefix3));
        BOOST_TEST(prefix3 != prefix2);   

        // bind an arbitrary address
        BOOST_TEST(resolver.bind(1, address(here, 1, 2)));
        
        // associate this id with a namespace name
        BOOST_TEST(resolver.registerid("/test/foo/1", 1));
        
        // resolve this address
        address addr;
        BOOST_TEST(resolver.resolve(1, addr));
        BOOST_TEST(addr == address(here, 1, 2));

        // try to resolve a non-existing address
        BOOST_TEST(!resolver.resolve(2, addr));

        // check association of the namespace name
        id_type id = 0;
        BOOST_TEST(resolver.queryid("/test/foo/1", id));
        BOOST_TEST(id == 1);
        
        // rebind the id above to a new address
        BOOST_TEST(!resolver.bind(1, address(here, 1, 3)));

        // re-associate this id with a namespace name
        BOOST_TEST(!resolver.registerid("/test/foo/1", 2));

        // resolve it again
        BOOST_TEST(resolver.resolve(1, addr));
        BOOST_TEST(addr == address(here, 1, 3));

        // re-check association of the namespace name
        BOOST_TEST(resolver.queryid("/test/foo/1", id));
        BOOST_TEST(id == 2);

        // unbind the address
        BOOST_TEST(resolver.unbind(1));

        // remove association
        BOOST_TEST(resolver.unregisterid("/test/foo/1"));
        
        // resolve should fail now
        BOOST_TEST(!resolver.resolve(1, addr));

        // association of the namespace name should fail now
        BOOST_TEST(!resolver.queryid("/test/foo/1", id));

        // repeated unbind should fail
        BOOST_TEST(!resolver.unbind(1));

        // repeated remove association should fail
        BOOST_TEST(!resolver.unregisterid("/test/foo/1"));
    }
    catch (std::exception& e) {
        std::cerr << "std::exception caught: " << e.what() << "\n";
    }
    catch (...) {
        std::cerr << "unexpected exception caught\n";
    }
    return boost::report_errors();
}

