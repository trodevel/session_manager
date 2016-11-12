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

// $Revision: 4978 $ $Date:: 2016-11-11 #$ $Author: serge $

#include "manager.h"            // session_manager::Manager
#include "i_authenticator.h"    // session_manager::IAuthenticator

#include <iostream>             // std::cout
#include <thread>               // std::this_thread

class Authenticator: public session_manager::IAuthenticator
{
public:

    Authenticator()
    {
        map_user_to_pwd_hash_.insert( Map::value_type( 1, std::hash<std::string>()( "alpha" ) ) );
        map_user_to_pwd_hash_.insert( Map::value_type( 2, std::hash<std::string>()( "beta" ) ) );
        map_user_to_pwd_hash_.insert( Map::value_type( 3, std::hash<std::string>()( "gamma" ) ) );
        map_user_to_pwd_hash_.insert( Map::value_type( 4, std::hash<std::string>()( "omega" ) ) );
    }

    // interface session_manager::IAuthenticator
    virtual bool is_authenticated( uint32_t user_id, const std::string & password ) const
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

    typedef std::map<uint32_t,std::size_t> Map;

    Map map_user_to_pwd_hash_;
};

void test_auth( session_manager::Manager & m, uint32_t user_id, const std::string & password )
{
    std::cout << "testing: user = " << user_id << ", password = " << password << std::endl;

    std::string id;
    std::string error;

    auto b = m.authenticate( user_id, password, id, error );

    if( b )
    {
        std::cout << "OK: user authenticated: session id = " << id << std::endl;
    }
    else
    {
        std::cout << "ERROR: " << error << std::endl;
    }
}

void test_is_auth( session_manager::Manager & m, uint32_t user_id, const std::string & password )
{
    std::cout << "testing: user = " << user_id << ", password = " << password << std::endl;

    std::string id;
    std::string error;

    auto b = m.authenticate( user_id, password, id, error );

    if( b )
    {
        std::cout << "OK: user authenticated: session id = " << id << std::endl;

        auto is_auth = m.is_authenticated( id );

        if( is_auth )
        {
            std::cout << "OK: session id authenticated" << std::endl;

            auto is_closed = m.close_session( id, error );

            if( is_closed )
            {
                std::cout << "OK: session successfully closed" << std::endl;
            }
            else
            {
                std::cout << "ERROR: " << error << std::endl;
            }
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

void test_is_auth_user( session_manager::Manager & m, uint32_t user_id, const std::string & password )
{
    std::cout << "testing: user = " << user_id << ", password = " << password << std::endl;

    std::string id;
    std::string error;

    auto b = m.authenticate( user_id, password, id, error );

    if( b )
    {
        std::cout << "OK: user authenticated: session id = " << id << std::endl;

        session_manager::Manager::user_id_t auth_user_id;

        auto is_auth = m.is_authenticated( id, auth_user_id );

        if( is_auth )
        {
            if( user_id == auth_user_id )
            {
                std::cout << "OK: session id authenticated for expected user " << auth_user_id << std::endl;
            }
            else
            {
                std::cout << "ERROR: session is authenticated for wrong user " << auth_user_id << std::endl;
            }
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

void test_wrong_id( session_manager::Manager & m )
{
    std::string id = "b3ef4ab3-2c1c-4590-8d9c-a764eb068dbf";

    auto is_auth = m.is_authenticated( id );

    if( is_auth )
    {
        std::cout << "ERROR: wrong session id was incorrectly authenticated" << std::endl;
    }
    else
    {
        std::cout << "OK: wrong session id was not authenticated" << std::endl;
    }
}

void test_close_wrong_id( session_manager::Manager & m )
{
    std::string id = "b3ef4ab3-2c1c-4590-8d9c-a764eb068dbf";

    std::string error;

    auto is_closed = m.close_session( id, error );

    if( is_closed )
    {
        std::cout << "ERROR: wrong session id was incorrectly closed" << std::endl;
    }
    else
    {
        std::cout << "OK: wrong session id was not closed: " << error << std::endl;
    }
}

void test_max_sessions( session_manager::Manager & m, uint32_t user_id, const std::string & password )
{
    test_auth( m, user_id, password );
    test_auth( m, user_id, password );
    test_auth( m, user_id, password );
}

void test_expiration( session_manager::Manager & m, uint32_t user_id, const std::string & password, uint32_t sleep )
{
    std::cout << "testing: user = " << user_id << ", password = " << password << std::endl;

    std::string id;
    std::string error;

    auto b = m.authenticate( user_id, password, id, error );

    if( b )
    {
        std::cout << "OK: user authenticated: session id = " << id << std::endl;

        auto is_auth = m.is_authenticated( id );

        if( is_auth )
        {
            std::cout << "OK: session id authenticated, sleep " << sleep << " min" << std::endl;

            std::chrono::minutes timespan( sleep );

            std::this_thread::sleep_for( timespan );

            auto is_auth2 = m.is_authenticated( id );

            if( is_auth2 )
            {
                std::cout << "ERROR: expired session id was authenticated" << std::endl;
            }
            else
            {
                std::cout << "OK: expired session id was NOT authenticated" << std::endl;
            }
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

void test_remove_expired( session_manager::Manager & m, uint32_t user_id, const std::string & password, uint32_t sleep )
{
    std::cout << "testing: user = " << user_id << ", password = " << password << std::endl;

    std::string id;
    std::string error;

    std::cout << "try: 1" << std::endl;

    auto b = m.authenticate( user_id, password, id, error );

    if( b == false )
    {
        std::cout << "ERROR: " << error << std::endl;
        return;
    }

    std::cout << "OK: user authenticated: session id = " << id << std::endl;

    std::cout << "try: 2" << std::endl;

    b = m.authenticate( user_id, password, id, error );

    if( b == false )
    {
        std::cout << "ERROR: " << error << std::endl;
        return;
    }

    std::cout << "OK: user authenticated: session id = " << id << std::endl;

    std::cout << "try: 3" << std::endl;

    b = m.authenticate( user_id, password, id, error );

    if( b == false )
    {
        std::cout << "ERROR: " << error << std::endl;

        std::cout << "OK: cannot authenticate, sleep " << sleep << " min" << std::endl;

        std::chrono::minutes timespan( sleep );

        std::this_thread::sleep_for( timespan );
    }

    std::cout << "try: 4" << std::endl;

    b = m.authenticate( user_id, password, id, error );

    if( b == false )
    {
        std::cout << "ERROR: " << error << std::endl;
        return;
    }

    std::cout << "OK: user authenticated: session id = " << id << std::endl;
}

int main()
{
    Authenticator a;

    session_manager::Manager m;

    session_manager::Manager::Config cfg;

    cfg.expiration_time         = 1;
    cfg.max_sessions_per_user   = 2;
    cfg.postpone_expiration     = true;

    auto b = m.init( & a, cfg );

    if( b == false )
    {
        std::cout << "ERROR: initialization failed" << std::endl;
        return 0;
    }

    const uint32_t user1 = 1;
    const uint32_t user2 = 2;
    const uint32_t user3 = 3;
    const uint32_t user4 = 4;

    test_auth( m, user1, "blabla" );
    test_auth( m, user1, "alpha" );

    test_is_auth( m, user2, "beta" );

    test_is_auth_user( m, user2, "beta" );

    test_max_sessions( m, user3, "gamma" );

    test_wrong_id( m );

    test_close_wrong_id( m );

    test_expiration( m, user2, "beta", cfg.expiration_time );

    test_remove_expired( m, user4, "omega", cfg.expiration_time );

    return 0;
}
