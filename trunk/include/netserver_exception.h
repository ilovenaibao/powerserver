/*
 * netserver_excpetion.h
 *
 *  Created on: 2009-7-15
 *      Author: root
 */

#ifndef NETSERVER_EXCPETION_H_
#define NETSERVER_EXCPETION_H_

#include <string>
#include <errno.h>
#include <string.h>

namespace exceptions
{
	class sockexception
	{
	public:
		inline sockexception(int errnumber)
		{
			errid = errnumber;
			strerror_r(errnumber, txterr, 508);
		}

		inline sockexception(int errnumber, const char * str)
		{
			errid = errnumber;
			strncpy(txterr, str, 508);
			txterr[507] = 0;
		}

		inline int what()
		{
			return errid;
		}

		inline int what() const
		{
			return errid;
		}

		inline const char * what_str()
		{
			return txterr;
		}

		inline const char * what_str() const
		{
			return txterr;
		}

	protected:
		error_t errid;
		char txterr[508];
	};
} // namespace netserver

#endif /* NETSERVER_EXCPETION_H_ */
