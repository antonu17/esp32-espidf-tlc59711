<template>
  <v-container>
    <v-row>
      <v-col>
        <Modes
          v-bind:loading="isLoading"
          v-bind:currentMode="currentMode"
          @switch-mode="switchMode"
        />
      </v-col>
      <v-col>
        <Effects
          v-bind:loading="isLoading"
          v-bind:effects="effects"
          v-bind:currentEffect="currentEffect"
          @switch-effect="switchEffect"
        />
      </v-col>
    </v-row>
  </v-container>
</template>

<script>
// @ is an alias to /src
import Effects from "@/components/Effects.vue";
import Modes from "@/components/Modes.vue";

export default {
  name: "Home",
  components: {
    Effects,
    Modes
  },
  computed: {
    isLoading() {
      return this.$store.getters.isLoading;
    },
    effects() {
      return this.$store.getters.effects;
    },
    currentEffect() {
      return this.$store.getters.currentEffect;
    },
    currentMode() {
      return this.$store.getters.currentMode;
    }
  },
  data() {
    return {
      timer: null
    };
  },
  methods: {
    updateData: function() {
      this.$store.dispatch("fetchCurrentMode");
    },
    switchEffect(effect) {
      this.$ajax
        .post("/api/v1/mode", {
          effect: effect
        })
        .then(data => {
          console.log(data);
        })
        .catch(error => {
          console.log(error);
        });
    },
    switchMode(mode) {
      this.$ajax
        .post("/api/v1/mode", {
          mode: mode
        })
        .then(data => {
          console.log(data);
        })
        .catch(error => {
          console.log(error);
        });
    }
  },
  mounted() {
    this.$store.dispatch("fetchEffects");
    clearInterval(this.timer);
    this.timer = setInterval(this.updateData, 2000);
  }
};
</script>
