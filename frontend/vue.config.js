module.exports = {
  "transpileDependencies": [
    "vuetify"
  ],
  devServer: {
    proxy: {
      '/api': {
        target: 'http://rgb-cube.local',
        changeOrigin: true,
        ws: true
      }
    }
  }
}
