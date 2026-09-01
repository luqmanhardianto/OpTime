#include <Arduino.h>

#include "core/EventSystem.h"

void setup()
{
    Serial.begin(9600);

    EventSystem eventSystem;
    eventSystem.begin();

    Event event = {EventType::BUTTON_SHORT, static_cast<uint8_t>(EventSource::BUTTON), 1};
    const StatusCode published = eventSystem.publish(event);
    if (published != StatusCode::OK)
    {
        Serial.println("Publish failed");
        return;
    }

    Event consumed = {};
    if (!eventSystem.consume(consumed))
    {
        Serial.println("Consume failed");
        return;
    }

    if (consumed.type != EventType::BUTTON_SHORT || consumed.source != static_cast<uint8_t>(EventSource::BUTTON) || consumed.value != 1)
    {
        Serial.println("Event mismatch");
        return;
    }

    Serial.println("Event system tests completed");
}

void loop()
{
}
