# Summary of Changes:

- Add _channels vector to Server.hpp
- Add all missing function declarations to Server.hpp
- Implement createChannel(), removeChannel(), getClientByNickname()
- Add PRIVMSG and NOTICE to executeCommand()
- Update destructor to clean up channels
- Add missing reply codes to Utils.hpp
- Fix typo in Join.cpp (splitbyComma → Utils::splitByComma)
- Add NOTICE to valid commands in Command.cpp
- Test JOIN and PRIVMSG commands thoroughly