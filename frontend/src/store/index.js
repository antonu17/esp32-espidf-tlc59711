import Vue from 'vue'
import Vuex from 'vuex'
const axios = require('axios').default;

Vue.use(Vuex)

export default new Vuex.Store({
  state: {
    loadingStatus: 'notLoading',
    effects: [],
    currentEffect: null,
    currentMode: null
  },
  mutations: {
    SET_LOADING_STATUS(state, status) {
      state.loadingStatus = status
    },
    SET_EFFECTS(state, effects) {
      state.effects = effects
    },
    SET_CURRENT_EFFECT(state, effect) {
      state.currentEffect = effect
    },
    SET_CURRENT_MODE(state, mode) {
      state.currentMode = mode
    }
  },
  actions: {
    fetchEffects(context) {
      context.commit('SET_LOADING_STATUS', 'loading')
      axios.get('/api/v1/effects')
        .then(function (response) {
          console.log(response);
          context.commit('SET_LOADING_STATUS', 'notLoading')
          context.commit('SET_EFFECTS', response.data.effects)
        })
        .catch(function (error) {
          console.log(error);
        })
    }
  },
  modules: {
  }
})
