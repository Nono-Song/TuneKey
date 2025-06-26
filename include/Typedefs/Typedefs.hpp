//
// Created by Huanming Song on 6/26/25.
//

#pragma once
#include <cstdint>
#include <string>
#include <filesystem>
#include <concepts>
#include <functional>

struct PlayEvent;
struct PauseEvent;
struct StopEvent;
struct ResumeEvent;
struct AudioReadyEvent;
struct AudioFinishedEvent;
struct AudioErrorEvent;
struct ShutdownEvent;

class Button;

/** Button attributes types **/
using identifier_type = uint64_t;
using name_type = std::string;
using filename_type = std::filesystem::path;
using timestamp_type = std::chrono::system_clock::time_point;

template <typename T> concept Identifier = std::same_as<T, identifier_type>;
template <typename T> concept NameType = std::same_as<T, name_type>;
template <typename T> concept FilenameType = std::same_as<T, filename_type>;
template <typename T> concept TimestampType = std::same_as<T, timestamp_type>;

template <typename T>
concept ButtonEvent =
    std::same_as<T, PlayEvent> ||
    std::same_as<T, PauseEvent> ||
    std::same_as<T, StopEvent> ||
    std::same_as<T, ResumeEvent>;

template <typename T>
concept ButtonAttr = NameType<T> || FilenameType<T> || Identifier<T> || TimestampType<T>;

template <typename T>
concept ModifiableAttr = NameType<T> || FilenameType<T>;
template <typename T>
concept ButtonAttrArg = std::assignable_from<name_type&, T> || std::assignable_from<filename_type&, T>;

/** Projector **/
template <ButtonAttr U>
using ButtonProjector = const U&(*)(const Button&);
using ButtonProjectorVariant = std::variant<
    ButtonProjector<name_type>,
    ButtonProjector<identifier_type>,
    ButtonProjector<filename_type>>;


/** Event related types **/
using Event = std::variant<PlayEvent,
                           PauseEvent,
                           ResumeEvent,
                           StopEvent,
                           AudioReadyEvent,
                           AudioFinishedEvent,
                           AudioErrorEvent,
                           ShutdownEvent>;

template <typename T>
concept EventType = std::same_as<T, PlayEvent> ||
                    std::same_as<T, ResumeEvent> ||
                    std::same_as<T, PauseEvent> ||
                    std::same_as<T, StopEvent> ||
                    std::same_as<T, AudioReadyEvent> ||
                    std::same_as<T, ShutdownEvent> ||
                    std::same_as<T, AudioFinishedEvent> ||
                    std::same_as<T, AudioErrorEvent>;