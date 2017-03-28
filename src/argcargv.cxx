int main ( int argc, const char** argv) {
  
    if ( argc ==  2 ) {
      std::vector<std::string> arguments( argv+1, argv+argc );
      leadJetPtMin = atof ( arguments[0].c_str() );
      std::cout << leadJetPtMin << std::endl;
    }
    else { __ERR( "incorrect number of command line arguments" ) return -1; }
