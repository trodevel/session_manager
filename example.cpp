/*

Example.

Copyright (C) 2016 Sergey Kolevatov

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

*/

// $Revision: 3583 $ $Date:: 2016-03-31 #$ $Author: serge $

#include "manager.h"            // session_manager::Manager
#include "i_authenticator.h"    // session_manager::IAuthenticator

#include <iostream>             // std::cout

class Authenticator: public session_manager::IAuthenticator
{
public:

    Authenticator()
    {
        map_user_to_pwd_hash_.insert( Map::value_type( "user1", std::hash<std::string>()( "alpha" ) ) );
        map_user_to_pwd_hash_.insert( Map::value_type( "user2", std::hash<std::string>()( "beta" ) ) );
        map_user_to_pwd_hash_.insert( Map::value_type( "user3", std::hash<std::string>()( "gamma" ) ) );
    }

    // interface session_manager::IAuthenticator
    virtual bool is_authenticated( const std::string & user_id, const std::string & password ) const
    {
        auto it = map_user_to_pwd_hash_.find( user_id );

        if( it == map_user_to_pwd_hash_.end() )
        {
            return false;
        }

        auto hash = std::hash<std::string>()( password );

        if( hash != it->second )
            return false;

        return true;
    }

private:

    typedef std::map<std::string,std::size_t> Map;

    Map map_user_to_pwd_hash_;
};

void test_auth( session_manager::Manager & m, const std::string & user_id, const std::string & password )
{
    std::cout << "testing: user = " << user_id << ", password = " << password << std::endl;

    std::string id;
    std::string error;

    auto b = m.authenticate( user_id, password, id, error );

    if( b )
    {
        std::cout << "authenticated: session id = " << id << std::endl;
    }
    else
    {
        std::cout << "ERROR: " << error << std::endl;
    }
}

void test_is_auth( session_manager::Manager & m, const std::string & user_id, const std::string & password )
{
    std::cout << "testing: user = " << user_id << ", password = " << password << std::endl;

    std::string id;
    std::string error;

    auto b = m.authenticate( user_id, password, id, error );

    if( b )
    {
        std::cout << "authenticated: session id = " << id << std::endl;

        auto is_auth = m.is_authenticated( id );

        if( is_auth )
        {
            std::cout << "session id authenticated" << std::endl;
        }
        else
        {
            std::cout << "ERROR: session id NOT authenticated" << std::endl;
        }
    }
    else
    {
        std::cout << "ERROR: " << error << std::endl;
    }
}

int main()
{
    Authenticator a;

    session_manager::Manager m;

    session_manager::Manager::Config cfg;

    cfg.expiration_time         = 5;
    cfg.max_sessions_per_user   = 2;
    cfg.postpone_expiration     = true;

    auto b = m.init( & a, cfg );

    if( b == false )
    {
        std::cout << "ERROR: initialization failed" << std::endl;
        return 0;
    }

    test_auth( m, "user1", "blabla" );
    test_auth( m, "user1", "alpha" );

    test_is_auth( m, "user2", "beta" );

    return 0;
}
