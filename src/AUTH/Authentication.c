/* Keywords:
EHLO -> AUTH

ARGUNENT:
mehcnism 
Si el mechanism no esta soportado, se debe responder con un 504
Si el cliente corta la conexion con "*" se debe responder con un 501
initial-response
*/

boolean Verify(string authzid, string authcid, string passwd) {
     string pAuthcid = SASLprep(authcid, true); # prepare authcid
     string pPasswd = SASLprep(passwd, true);   # prepare passwd
     if (pAuthcid == NULL || pPasswd == NULL) {
       return false;     # preparation failed
     }
     if (pAuthcid == "" || pPasswd == "") {
       return false;     # empty prepared string
     }




Zeilenga                    Standards Track                     [Page 4]

RFC 4616                The PLAIN SASL Mechanism             August 2006


     storedHash = FetchPasswordHash(pAuthcid);
     if (storedHash == NULL || storedHash == "") {
       return false;     # error or unknown authcid
     }

     if (!Compare(storedHash, Hash(pPasswd))) {
       return false;     # incorrect password
     }

     if (authzid == NULL ) {
       authzid = DeriveAuthzid(pAuthcid);
       if (authzid == NULL || authzid == "") {
           return false; # could not derive authzid
       }
     }

     if (!Authorize(pAuthcid, authzid)) {
       return false;     # not authorized
     }

     return true;
   }