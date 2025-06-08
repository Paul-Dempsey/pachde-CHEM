#include "midi-devices.hpp"

namespace pachde {

bool ExcludeDriver(const std::string & name)
{
    auto text = to_lower_case(name);
    if (0 == text.compare(0, 7, "gamepad", 0, 7)) {
        return true;
    }
    if (0 == text.compare(0, 8, "loopback", 0, 8)) {
        return true;
    }
    return std::string::npos != text.find("keyboard");
}

bool ExcludeDevice(const std::string & name)
{
#if defined ARCH_WIN
    return (name.size() >= 28) && (0 == name.compare(0, 28, "Microsoft GS Wavetable Synth", 0, 28));
#else
    return false;
#endif
}

bool is_EMDevice(const std::string& name)
{
    if (name.empty()) { return false; }
    auto text = to_lower_case(name);
    // Continuum <serial> and ContinuuMini <serial>
    if (0 == text.compare(0, 9, "continuum", 0, 9)) {
        return true;
    }
    // "EaganMatrix Module"
    // "EaganMatrix Micro <serial>"
    if (0 == text.compare(0, 11, "eaganmatrix", 0, 11)) {
        return true;
    }
    // Osmose varies depending on OS, and must be Osmose port 2
    return std::string::npos != text.find_first_of('2')
        && std::string::npos != text.find("osmose");
}

bool is_osmose(const std::string& name)
{
    if (name.empty()) { return false; }
    if (std::string::npos == name.find_first_of('2')) return false;
    auto text = to_lower_case(name);
    return std::string::npos != text.find("osmose");
}

#if defined ARCH_MAC
std::string FilterDeviceName(const std::string &raw) { return raw; }
#endif

#if defined ARCH_WIN
std::string FilterDeviceName(const std::string &raw)
{
    if (raw.empty()) return raw;
    std::string text = raw;
    text.erase(text.find_last_not_of("0123456789"));
    return text;
}
#endif

#if defined ARCH_LIN
std::string FilterDeviceName(const std::string &raw)
{
    if (raw.empty()) return raw;
    std::string text = raw;
    auto pos = text.find(':');
    if (std::string::npos != pos)
    {
        return text.substr(0, pos);
    }
    return text;
}
#endif

bool matchInOut(const std::string& input, const std::string& output)
{
    if (0 == input.compare(output)) return true;
#if defined ARCH_WIN
    return (0 == input.compare(0,7, "MIDIIN2", 0,7))
        && (0 == output.compare(0,8, "MIDIOUT2", 0,8)) 
        && std::string::npos != input.find("Osmose")
        && std::string::npos != output.find("Osmose");
#endif
    return false;
}

bool MidiDeviceConnectionInfo::parse(const std::string & spec)
{
    clear();
    if (spec.empty()) return false;
    int seq = 0;
    int state = 0;
    for (auto ch: spec) {
        if (CLAIM_SEPARATOR == ch) {
            ++state;
            continue;
        }
        switch (state) {
            case 0: input_device_name.push_back(ch); break;
            case 1: output_device_name.push_back(ch); break;
            case 2: driver_name.push_back(ch); break;
            case 3:
                if (std::isdigit(ch)) {
                    seq = (seq * 10) + (ch - '0');
                } else {
                    clear();
                    return false;
                }
                break;
            default:
                clear();
                return false;
        }
    }
    sequence = seq;
    return true;
}

std::string MidiDeviceConnectionInfo::claim() const
{
    if (claim_spec.empty()) {
        auto s = input_device_name;
        s.push_back(CLAIM_SEPARATOR);
        s.append(output_device_name);
        s.push_back(CLAIM_SEPARATOR);
        s.append(driver_name);
        s.push_back(CLAIM_SEPARATOR);
        if (sequence < 10) {
            s.push_back('0' + sequence);
        } else {
            s.append(format_string("%d", sequence));
        }
        const_cast<MidiDeviceConnectionInfo*>(this)->claim_spec = s;
    }
    return claim_spec;
}

std::string MidiDeviceConnectionInfo::friendly(TextFormatLength length) const
{
    if (input_device_name.empty()) return "(Unknown)";
    std::string result = input_device_name;

    switch (length) {
    case TextFormatLength::Short: {
        if (0 == result.compare("MIDIIN2 (Osmose)")) {
            result = "Osmose";
        }
    } break;

    case TextFormatLength::Compact: {
        if (0 == result.compare(0, 9, "Continuum", 0, 9)) {
            result.replace(0, 10, "Cont-");
        } else if (0 == result.compare(0, 12, "ContinuuMini", 0, 12)) {
            result.erase(0, 8); // "Mini"
        } else if (0 == result.compare("EaganMatrix Module")) {
            result.replace(0, 18, "EMM");
        } else if (0 == result.compare("MIDIIN2 (Osmose)")) {
            result = "Osmose";
        }
    } break;

    case TextFormatLength::Abbreviated:{
        if (0 == result.compare(0, 9, "Continuum", 0, 9)) {
            result.replace(1, 9, "C-"); // "C-"
        } else if (0 == result.compare(0, 12, "ContinuuMini", 0, 12)) {
            result.replace(0, 12, "M-"); // substitute "M-"
        } else if (0 == result.compare(0, 18, "EaganMatrix Module", 0 , 18)) {
            result.replace(0, 18, "EMM");
        } else if (0 == result.compare("MIDIIN2 (Osmose)")) {
            result = "Os";
        }
    } break;

    default:
        break;
    }

    if (sequence > 0) {
        result.append(format_string("#%d", sequence));
    }
    
    if (TextFormatLength::Long == length) {
        if (!output_device_name.empty()) {
            result.append(" and ");
            result.append(output_device_name);
        }
        if (!driver_name.empty()) {
            result.append(" on ");
            result.append(driver_name);
        }
    }
    return result;
}

std::vector<std::shared_ptr<MidiDeviceConnection>> EnumerateMidiConnections(bool emOnly)
{
    std::vector<std::shared_ptr<MidiDeviceConnection>> result;
    std::map<std::string, int> counts;

    for (auto id_driver : midi::getDriverIds())
    {
        auto driver = midi::getDriver(id_driver);
        if (emOnly && ExcludeDriver(driver->getName())) {
            continue;
        }

        // collect inputs
        for (auto id_input: driver->getInputDeviceIds()) {
            auto input_name = FilterDeviceName(driver->getInputDeviceName(id_input));
            if (emOnly && !is_EMDevice(input_name)) {
                continue;
            }
            auto item = std::make_shared<MidiDeviceConnection>();
            item->driver_id = id_driver;
            item->input_device_id = id_input;
            item->info.driver_name = driver->getName();
            item->info.input_device_name = input_name;
            auto r = counts.insert(std::make_pair(input_name, 0));
            if (!r.second) {
                r.first->second++;
            }
            item->info.sequence = r.first->second;
            result.push_back(item);
        }

        // match outputs to inputs
        counts.clear();
        for (auto id_out: driver->getOutputDeviceIds()) {
            auto output_name = FilterDeviceName(driver->getOutputDeviceName(id_out));
            if (ExcludeDevice(output_name) || (emOnly && !is_EMDevice(output_name))) {
                continue;
            }
            auto r = counts.insert(std::make_pair(output_name, 0));
            if (!r.second) {
                r.first->second++;
            }
            int seq = r.first->second;
            auto item = std::find_if(result.begin(), result.end(), [&](std::shared_ptr<MidiDeviceConnection>& item) {
                return matchInOut(item->info.input_device_name, output_name) &&
                    item->info.sequence == seq;
            });
            if (item != result.end()) {
                (*item)->output_device_id = id_out;
                if (output_name != (*item)->info.input_device_name) {
                    (*item)->info.output_device_name = output_name;
                }
            } 
            // else {
            //     DEBUG("No match for output device %s:%s:%d", driver->getName().c_str(), output_name.c_str(), seq);
            // }
        }
    }
    return result;
}




}