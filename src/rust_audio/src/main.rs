use rust_audio::{engine::RealtimeEngine, models};
use std::env;

fn main() -> anyhow::Result<()> {
    let args: Vec<String> = env::args().collect();
    if args.len() < 2 {
        eprintln!("Usage: {} <track.json>", args[0]);
        std::process::exit(1);
    }
    let track = models::load_track(std::path::Path::new(&args[1]))?;
    let mut engine = RealtimeEngine::new(track);
    engine.prepare();
    engine.play()?;
    Ok(())
}
