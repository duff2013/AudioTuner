void handleCmds( String cmd ) {
    String p = cmd;
    
    if (p.startsWith("f ")) {
        p.trim();
        p = p.substring(2);
        float t = p.toFloat();
        Serial.print("new frequency: ");
        Serial.println(t);
        AudioNoInterrupts();  // disable audio library momentarily
        sine.frequency(p.toFloat());
        AudioInterrupts();    // enable, both tones will start together
    }
    else if (p.startsWith("a ")) {
        p.trim();
        p = p.substring(2);
        float t = p.toFloat();
        Serial.print("new amplitude: ");
        Serial.println(t);
        AudioNoInterrupts();  // disable audio library momentarily
        sine.amplitude(p.toFloat());
        AudioInterrupts();    // enable, both tones will start together
    }
}
