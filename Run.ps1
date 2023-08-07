param(
  [Parameter(Mandatory)]
  [String]$gameMode
)

switch -Exact ($gameMode)
{
    "classic" {
        .\x64\Debug\AutoSudoku.exe `
            --game-mode $gameMode `
            --fill-order row `
            --play-interval 1000 `
            --logtostderr=1 `
            --minloglevel=0 `
            --stderrthreshold=0
    }
    "irregular" {
        .\x64\Debug\AutoSudoku.exe `
            --game-mode $gameMode `
            --fill-order block `
            --play-interval 1000 `
            --logtostderr=1 `
            --minloglevel=0 `
            --stderrthreshold=0
    }
    'icebreaker' {
        .\x64\Debug\AutoSudoku.exe `
            --game-mode $gameMode `
            --logtostderr=1 `
            --minloglevel=0 `
            --stderrthreshold=0
    }
    default {
        Write-Error "Unknown mode $gameMode"
    }
}
