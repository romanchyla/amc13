
#include <iostream>

#include "boost/python.hpp"
#include "boost/python/module.hpp"
#include "boost/python/def.hpp"
#include "boost/python/overloads.hpp"
#include "boost/python/wrapper.hpp"

#include "boost/noncopyable.hpp"

#include "amc13/AMC13.hh"
#include "amc13/python/converters.hpp"
#include "amc13/python/exceptions.hpp"


using namespace boost::python;

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(amc13_Flash_programFlash_overloads, programFlash, 1, 2);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(amc13_Flash_verifyFlash_overloads, verifyFlash, 1, 2);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(amc13_Status_Report_overloads, Report, 1, 3);


// *** N.B: The argument of this BOOST_PYTHON_MODULE macro MUST be the same as the name of the library created, i.e. if creating library file my_py_binds_module.so , imported in python as:
//                import my_py_binds_module
//          then would have to put
//                BOOST_PYTHON_MODULE(my_py_binds_module)
//          Otherwise, will get the error message "ImportError: dynamic module does not define init function (initmy_py_binds_module)

BOOST_PYTHON_MODULE(_amc13_python) {
  // CONVERTERS
  amc13py::register_converters();

  // EXCEPTIONS
  amc13py::wrap_exceptions();


  {
    scope amc13_AMC13Simple_scope = class_<amc13::AMC13Simple, boost::noncopyable>("AMC13Simple", no_init)
      .def("getT1", &amc13::AMC13Simple::getT1, return_internal_reference<>())
      .def("getT2", &amc13::AMC13Simple::getT2, return_internal_reference<>())
      .def("getChip", &amc13::AMC13Simple::getChip, return_internal_reference<>())
      .def("read",static_cast<uint32_t (amc13::AMC13Simple::*) (amc13::AMC13Simple::Board, const std::string&) >(&amc13::AMC13Simple::read))
      .def("read",static_cast<uint32_t (amc13::AMC13Simple::*) (amc13::AMC13Simple::Board, uint32_t) >(&amc13::AMC13Simple::read))
      .def("write",static_cast<void (amc13::AMC13Simple::*) (amc13::AMC13Simple::Board, const std::string&, uint32_t) >(&amc13::AMC13Simple::write))
      .def("write",static_cast<void (amc13::AMC13Simple::*) (amc13::AMC13Simple::Board, uint32_t, uint32_t) >(&amc13::AMC13Simple::write))
    ;

    enum_<amc13::AMC13Simple::Board>("Board")
      .value("UNKNOWN", amc13::AMC13Simple::UNKNOWN)
      .value("T1", amc13::AMC13Simple::T1)
      .value("T2", amc13::AMC13Simple::T2)
      .value("spartan", amc13::AMC13Simple::spartan)
      .value("virtex", amc13::AMC13Simple::virtex)
      .value("kintex", amc13::AMC13Simple::kintex);
  }

  class_<amc13::Flash, boost::noncopyable>("Flash", no_init)
    .def("readFlashPage", &amc13::Flash::readFlashPage)
    .def("firmwareFromFlash", &amc13::Flash::firmwareFromFlash)
// Not defined    .def("writeFlashPage", &amc13::Flash::writeFlashPage)
    .def("eraseFlashSector", &amc13::Flash::eraseFlashSector)
//    .def("programFlash", &amc13::Flash::programFlash, amc13_Flash_programFlash_overloads())
//    .def("verifyFlash", &amc13::Flash::verifyFlash, amc13_Flash_verifyFlash_overloads())
    .def("loadFlashT1", &amc13::Flash::loadFlashT1)
    .def("loadFlash", &amc13::Flash::loadFlash)
    .def("parseMcsFile", &amc13::Flash::parseMcsFile)
    .def("selectMcsFile", &amc13::Flash::selectMcsFile)
    .def("offset", &amc13::Flash::offset)
    .def("chipTypeFromSN", &amc13::Flash::chipTypeFromSN)
    .def("clearThrow", &amc13::Flash::clearThrow)
    .def("clear", &amc13::Flash::clear);

  class_<amc13::Status, boost::noncopyable>("Status", no_init)
    .def("Report", &amc13::Status::Report, amc13_Status_Report_overloads())
    .def("SetHTML",&amc13::Status::SetHTML)
    .def("UnsetHTML",&amc13::Status::UnsetHTML)
    .def("SetLaTeX",&amc13::Status::SetLaTeX)
    .def("UnsetLaTeX",&amc13::Status::UnsetLaTeX)
  ;


  class_<amc13::AMC13, bases<amc13::AMC13Simple>,  boost::noncopyable >("AMC13", init<const std::string&>())
    .def( init<const uhal::HwInterface& , const uhal::HwInterface& >() )
    .def( init<const std::string&, const std::string&, const std::string&>() )
    .def( init<const std::string&, const std::string&, const std::string&, const std::string&>() )
    .def("getFlash", &amc13::AMC13::getFlash, return_internal_reference<>())
    .def("getStatus", &amc13::AMC13::getStatus, return_internal_reference<>())
    .def("reset", &amc13::AMC13::reset)
    .def("resetCounters", &amc13::AMC13::resetCounters)
    .def("resetDAQ", &amc13::AMC13::resetDAQ)
    .def("AMCInputEnable", &amc13::AMC13::AMCInputEnable)
    .def("parseInputEnableList", &amc13::AMC13::parseInputEnableList)
    .def("enableAllTTC", &amc13::AMC13::enableAllTTC)
    .def("daqLinkEnable", &amc13::AMC13::daqLinkEnable)
    .def("fakeDataEnable", &amc13::AMC13::fakeDataEnable)
    .def("localTtcSignalEnable", &amc13::AMC13::localTtcSignalEnable)
    .def("configureLocalL1A", &amc13::AMC13::configureLocalL1A)
    .def("enableLocalL1A", &amc13::AMC13::enableLocalL1A)
    .def("startContinuousL1A", &amc13::AMC13::startContinuousL1A)
    .def("stopContinuousL1A", &amc13::AMC13::stopContinuousL1A)
    .def("sendL1ABurst", &amc13::AMC13::sendL1ABurst)
    .def("monBufBackPressEnable", &amc13::AMC13::monBufBackPressEnable)
    .def("setOrbitGap", &amc13::AMC13::setOrbitGap)
    .def("sendLocalEvnOrnReset", &amc13::AMC13::sendLocalEvnOrnReset)
//    .def("setOcrCommand", static_cast<void (amc13::AMC13::*)( uint32_t )>(&amc13::AMC13::setOcrCommand) )
    .def("setOcrCommand", static_cast<void (amc13::AMC13::*)(uint32_t) >(&amc13::AMC13::setOcrCommand))
    .def("setOcrCommand", static_cast<void (amc13::AMC13::*)(uint32_t, uint32_t) >(&amc13::AMC13::setOcrCommand))
    .def("setResyncCommand", static_cast<void (amc13::AMC13::*)(uint32_t) >(&amc13::AMC13::setResyncCommand))
    .def("setResyncCommand", static_cast<void (amc13::AMC13::*)(uint32_t, uint32_t) >(&amc13::AMC13::setResyncCommand))
    .def("configurePrescale", &amc13::AMC13::configurePrescale)
    .def("configureBGOShort", &amc13::AMC13::configureBGOShort)
    .def("configureBGOLong", &amc13::AMC13::configureBGOLong)
    .def("enableBGO", &amc13::AMC13::enableBGO)
    .def("disableBGO", &amc13::AMC13::disableBGO)
    //.def("sendBGO", &amc13::AMC13::sendBGO)

    .def("setFEDid", static_cast<void (amc13::AMC13::*)(uint32_t) > (&amc13::AMC13::setFEDid))
    .def("setFEDid", static_cast<void (amc13::AMC13::*)(int,uint32_t) > (&amc13::AMC13::setFEDid))

    .def("setSlinkID", &amc13::AMC13::setSlinkID)
    .def("setBcnOffset", &amc13::AMC13::setBcnOffset)
    .def("ttsDisableMask", &amc13::AMC13::ttsDisableMask)
    .def("sfpOutputEnable", &amc13::AMC13::sfpOutputEnable)
    .def("startRun", &amc13::AMC13::startRun)
    .def("endRun", &amc13::AMC13::endRun)
    .def("readEvent", static_cast<std::vector<uint64_t> (amc13::AMC13::*) ()>(&amc13::AMC13::readEvent))
    .def("GetEnabledAMCMask", &amc13::AMC13::GetEnabledAMCMask)
    .def("setTTCHistoryEna", &amc13::AMC13::setTTCHistoryEna)
    .def("setTTCFilterEna", &amc13::AMC13::setTTCFilterEna)
    .def("setTTCHistoryFilter", &amc13::AMC13::setTTCHistoryFilter)
    .def("clearTTCHistoryFilter", &amc13::AMC13::clearTTCHistoryFilter)
    .def("clearTTCHistory", &amc13::AMC13::clearTTCHistory)
    // .def("getTTCHistory", &amc13::AMC13::getTTCHistory)
    .def("GetVersion", &amc13::AMC13::GetVersion)
    .def("calTrigEnable", &amc13::AMC13::calTrigEnable)
    .def("getCalTrigWindowHigh", &amc13::AMC13::getCalTrigWindowHigh)
    .def("getCalTrigWindowLow", &amc13::AMC13::getCalTrigWindowLow)  
    .def("getL1AHistory", &amc13::AMC13::getL1AHistory)  
    ;

}
