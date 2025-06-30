use realtime_audio_rs::{RealtimeEngine, RealtimeSynthContext, Track};
use std::fs;

fn main() -> anyhow::Result<()> {
    let args: Vec<String> = std::env::args().collect();
    if args.len() < 2 {
        println!("Usage: realtime_audio_rs <track.json>");
        return Ok(());
    }
    let data = fs::read_to_string(&args[1])?;
    let track: Track = serde_json::from_str(&data)?;
    let mut engine = RealtimeEngine::new(RealtimeSynthContext::default());
    engine.load_track(track);
    engine.play()?;
    Ok(())
}
