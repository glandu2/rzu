#include "DbQueryJob.h"
#include <stdarg.h>
#include <string.h>
#include "EventLoop.h"
#include "Log.h"
#include "DbConnectionPool.h"

DbQueryBinding::DbQueryBinding(DbConnectionPool* dbConnectionPool,
					   cval<std::string>& connectionString,
					   cval<std::string>& query,
					   const std::vector<ParameterBinding>& parameterBindings,
					   const std::vector<ColumnBinding>& columnBindings)
	: dbConnectionPool(dbConnectionPool),
	  connectionString(connectionString),
	  query(query),
	  parameterBindings(parameterBindings),
	  columnBindings(columnBindings)
{
}

DbQueryBinding::~DbQueryBinding() {
}

bool DbQueryBinding::process(IDbQueryJob* queryJob, void* instance, ExecuteMode mode) {
	DbConnection* connection;
	bool columnCountOk;

	connection = dbConnectionPool->getConnection(connectionString.get().c_str());
	if(!connection) {
		warn("Could not retrieve a DB connection from pool\n");
		return false;
	}

	std::string queryStr = query.get();

	trace("Executing: %s\n", queryStr.c_str());

	for(size_t i = 0; i < parameterBindings.size(); i++) {
		const ParameterBinding& paramBinding = parameterBindings.at(i);

		if(paramBinding.index > 0) {
			if(paramBinding.isStdString) {
				std::string* str = (std::string*) ((char*)instance + paramBinding.bufferOffset);
				connection->bindParameter(paramBinding.index, SQL_PARAM_INPUT,
										  paramBinding.cType,
										  paramBinding.dbType, str->size(), paramBinding.dbPrecision,
										  (SQLPOINTER)str->c_str(), str->size(),
										  paramBinding.infoPtr);
			} else {
				connection->bindParameter(paramBinding.index, SQL_PARAM_INPUT,
										  paramBinding.cType,
										  paramBinding.dbType, paramBinding.dbSize, paramBinding.dbPrecision,
										  (char*)instance + paramBinding.bufferOffset, paramBinding.bufferSize,
										  paramBinding.infoPtr);
			}
			//TODO: print content of params buffer
		}
	}

	if(!connection->execute(queryStr.c_str())) {
		connection->releaseWithError();
		debug("DB query failed: %s\n", queryStr.c_str());
		return false;
	}

	if(mode != EM_NoRow) {
		bool firstRowFetched = false;
		while(connection->fetch() && (firstRowFetched == false || mode == EM_MultiRows)) {
			firstRowFetched = true;

			trace("Fetching data\n");
			const int columnCount = connection->getColumnNum(&columnCountOk);

			if(!columnCountOk) {
				connection->releaseWithError();
				return false;
			}

			char columnName[32];
			for(int i = 0; i < columnCount; i++) {
				const int columnIndex = i + 1;
				SQLLEN StrLen_Or_Ind;

				connection->getColumnName(columnIndex, columnName, sizeof(columnName));

				//TODO: cache column to get indices
				for(size_t i = 0; i < columnBindings.size(); i++) {
					const ColumnBinding& columnBinding = columnBindings.at(i);

					if(!strcmp(columnName, columnBinding.name.get().c_str())) {
						if(columnBinding.isStdString)
						{
							std::string* str = (std::string*) ((char*)instance + columnBinding.bufferOffset);
							connection->getData(columnIndex, columnBinding.cType,
												&str[0], str->size(), &StrLen_Or_Ind);
							if(StrLen_Or_Ind != SQL_NULL_DATA && StrLen_Or_Ind != SQL_NO_TOTAL && (int)str->size() > StrLen_Or_Ind)
								str->resize(StrLen_Or_Ind);
						} else {
							connection->getData(columnIndex, columnBinding.cType,
												(char*)instance + columnBinding.bufferOffset, columnBinding.bufferSize, &StrLen_Or_Ind);
						}

						if(columnBinding.infoPtr)
							*columnBinding.infoPtr = StrLen_Or_Ind;
					}
				}
			}

			if(queryJob->onRowDone() == false)
				break;
		}
	}

	connection->release();

	return true;
}