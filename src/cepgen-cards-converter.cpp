#include "CepGen/Cards/Handler.h"
#include "CepGen/Core/Exception.h"
#include "CepGen/Generator.h"
#include "CepGen/Parameters.h"
#include "CepGen/Utils/ArgumentsParser.h"
#include "CepGen/Utils/Filesystem.h"

using namespace std;

int main(int argc, char* argv[]) {
  string input_config, output_config;
  bool debug;

  cepgen::ArgumentsParser parser(argc, argv);
  parser.addArgument("input,i", "input configuration", &input_config)
      .addArgument("output,o", "output output", &output_config)
      .addOptionalArgument("debug,d", "debugging mode", &debug, false)
      .parse();

  if (debug)
    cepgen::utils::Logger::get().level = cepgen::utils::Logger::Level::debug;

  cepgen::initialise();

  try {
    auto params = cepgen::card::Handler::parse(input_config);
    cepgen::card::Handler::write(params, output_config);
    CG_INFO("main") << "Successfully converted the \"" << cepgen::utils::fileExtension(input_config)
                    << "\" card into a \"" << cepgen::utils::fileExtension(output_config) << "\" card.\n\t"
                    << "\"" << output_config << "\" file created.";

  } catch (const cepgen::Exception& e) {
    throw CG_FATAL("main") << "Failed to convert a \"" << cepgen::utils::fileExtension(input_config)
                           << "\" card into a \"" << cepgen::utils::fileExtension(output_config) << "\" card!\n"
                           << e.message();
  }

  return 0;
}
