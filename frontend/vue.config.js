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
  },
  configureWebpack: {
    output: {
      filename: '[id].bundle.js'
    },
    optimization: {
      minimize: true,
      splitChunks: {
        maxSize: 200000,
        cacheGroups: {
          vendors: {
            test: /[\\/]node_modules[\\/]/,
            name() {
              return `v`;
            }
          }
        }
      }
    }
  }
}
